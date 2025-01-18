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

namespace sapphire_plugins::tube_unit
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
    TubeUnitEditor &editor;
    IdleTimer(TubeUnitEditor &e) : editor(e) {}
    void timerCallback() override { editor.idle(); }
};

TubeUnitEditor::TubeUnitEditor(shared::audioToUIQueue_t &atou, shared::uiToAudioQueue_T &utoa,
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

    auto bg = shared::getSvgForPath("libs/sapphire/res/tubeunit.svg");
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

    airflow = shared::makeLargeKnob(this, "tube_unit", "airflow_knob");
    shared::set_control_position(*airflow, 15, 15, 0.5, 0.5);
    shared::bindSlider(this, airflow, patchCopy.airflow);
#if 0
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
#endif

    setSize(300, 600);
    resized();

    idleTimer = std::make_unique<IdleTimer>(*this);
    idleTimer->startTimer(1000. / 60.);

    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, true});
}

TubeUnitEditor::~TubeUnitEditor()
{
    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, false});
    idleTimer->stopTimer();
}

void TubeUnitEditor::resized()
{
    if (background)
    {
        background->setTransformToFit(getLocalBounds().toFloat(), juce::RectanglePlacement());
    }
}

void TubeUnitEditor::idle() { shared::drainQueueFromUI(*this); }

} // namespace sapphire_plugins::tube_unit
