/*
 * Sapphire Plugins
 *
 * Bringing the magical world of CosineKitty's sapphire plugins for rack to your DAW
 *
 * Copyright 2024-2025, Don Cross, Paul Walker, Morgon Kanter and other authors, as
 * described in the github transaction log.
 *
 * This project is distributed under the Gnu General Public License, version 3.0 or later.
 * You can find the LICENSE file at the address below.
 *
 * The source code and license are at https://github.com/baconpaul/sapphire-plugins
 */

#include "editor.h"
#include "configuration.h"
#include "shared/graphics_resources.h"
#include "sapphire_panel.hpp"

#include <clap/plugin.h>

namespace sapphire_plugins::elastika
{

// Helper function for placing a control exactly. Juce positioning doesn't take subpixel locations,
// unlike the rest of Juce. So apply a rounded version of that and add a transform that shunts it to
// the exact right location.
//
// cx and cy are the coordinates from the SVG. These are, for whatever reason, slightly off when
// used on top of our background. dx and dy are the correction for that off-ness (determined
// experimentally).
void set_control_position(juce::Component &control, float cx, float cy, float dx, float dy)
{
    juce::Point<float> real{cx + dx, cy + dy};
    juce::Point<int> rounded = real.toInt();
    control.setCentrePosition(rounded);
    control.setTransform(juce::AffineTransform::translation(real.getX() - rounded.getX(),
                                                            real.getY() - rounded.getY()));
}

struct IdleTimer : juce::Timer
{
    ElastikaEditor &editor;
    IdleTimer(ElastikaEditor &e) : editor(e) {}
    void timerCallback() override { editor.idle(); }
};

ElastikaEditor::ElastikaEditor(shared::audioToUIQueue_t &atou, shared::uiToAudioQueue_T &utoa,
                               std::function<void()> flushOperator)
    : audioToUI(atou), uiToAudio(utoa), flushOperator(flushOperator)
{
    // Process any events we have
    idle();

    auto knob_xml = juce::XmlDocument::parse(*shared::getSvgForPath("res/knob_graphics/knob.svg"));
    auto marker_xml =
        juce::XmlDocument::parse(*shared::getSvgForPath("res/knob_graphics/knob-marker.svg"));
    lnf = std::make_unique<shared::LookAndFeel>(juce::Drawable::createFromSVG(*knob_xml),
                                                juce::Drawable::createFromSVG(*marker_xml));

    auto bg = shared::getSvgForPath("libs/sapphire/res/elastika.svg");
    if (bg.has_value())
    {
        auto bgx = juce::XmlDocument::parse(*bg);
        if (bgx)
        {
            background = juce::Drawable::createFromSVG(*bgx);
            background->setInterceptsMouseClicks(false, true);

            addAndMakeVisible(*background);
        }
    }

    input_tilt_knob = make_large_knob("input_tilt_knob");
    shared::bindSlider(this, input_tilt_knob, patchCopy.inputTilt);

    output_tilt_knob = make_large_knob("output_tilt_knob");
    shared::bindSlider(this, output_tilt_knob, patchCopy.outputTilt);

    drive_knob = make_large_knob("drive_knob");
    shared::bindSlider(this, drive_knob, patchCopy.drive);

    level_knob = make_large_knob("level_knob");
    shared::bindSlider(this, level_knob, patchCopy.level);

    fric_slider = make_slider("fric_slider");
    shared::bindSlider(this, fric_slider, patchCopy.friction);

    curl_slider = make_slider("curl_slider");
    shared::bindSlider(this, curl_slider, patchCopy.curl);

    span_slider = make_slider("span_slider");
    shared::bindSlider(this, span_slider, patchCopy.span);

    mass_slider = make_slider("mass_slider");
    shared::bindSlider(this, mass_slider, patchCopy.mass);

    stif_slider = make_slider("stif_slider");
    shared::bindSlider(this, stif_slider, patchCopy.stiffness);

    setSize(300, 600);
    resized();

    idleTimer = std::make_unique<IdleTimer>(*this);
    idleTimer->startTimer(1000. / 60.);

    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, true});
}

ElastikaEditor::~ElastikaEditor()
{
    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, false});
    idleTimer->stopTimer();
}

void ElastikaEditor::resized()
{
    if (background)
    {
        background->setTransformToFit(getLocalBounds().toFloat(), juce::RectanglePlacement());
    }
}

void ElastikaEditor::idle() { shared::drainQueueFromUI(*this); }

std::unique_ptr<juce::Slider> ElastikaEditor::make_large_knob(const std::string &pos)
{
    auto r = Sapphire::FindComponent("elastika", pos);
    auto cx = r.cx;
    auto cy = r.cy;

    static constexpr float dx = 0.5f;
    static constexpr float dy = 0.5f;
    auto kn = std::make_unique<juce::Slider>();
    kn->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kn->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    kn->setPopupMenuEnabled(true);
    kn->setSize(11, 11);
    kn->setRange(0, 1);
    kn->setValue(0.5);
    kn->setMouseDragSensitivity(100);
    kn->setDoubleClickReturnValue(true, 0.5);
    kn->setLookAndFeel(lnf.get());

    background->addAndMakeVisible(*kn);
    set_control_position(*kn, cx, cy, dx, dy);

    return kn;
}

std::unique_ptr<juce::Slider> ElastikaEditor::make_slider(const std::string &pos)
{
    auto r = Sapphire::FindComponent("elastika", pos);
    auto cx = r.cx;
    auto cy = r.cy;

    static constexpr float dx = 0.6875f;
    static constexpr float dy = 0.6875f;
    auto sl = std::make_unique<juce::Slider>();
    sl->setSliderStyle(juce::Slider::LinearVertical);
    sl->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sl->setSize(8, 28);
    sl->setMouseDragSensitivity(28);
    sl->setRange(0, 1);
    sl->setValue(0.5);
    sl->setDoubleClickReturnValue(true, 0.5);
    sl->setLookAndFeel(lnf.get());
    // TODO: Get the "snap to mouse position" to work with the scaling we have where we only use 90%
    // of the track (the remaining 10% is for the bottom part of the thumb; the thumb's "position"
    // is the very top pixel of the thumb). Until then, it doesn't work right throughout the whole
    // track, so we set this to false.
    sl->setSliderSnapsToMousePosition(false);

    background->addAndMakeVisible(*sl);
    set_control_position(*sl, cx, cy, dx, dy);
    return sl;
}

} // namespace sapphire_plugins::elastika