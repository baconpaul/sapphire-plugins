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

    // FIXFIXFIX: text labels live inside tubeunit_labels.svg.
    // FIXFIXFIX: "vent" lives inside tubeunit_vent.svg.
    // FIXFIXFIX: "seal" lives inside tubeunit_seal.svg.

    airflow = shared::makeLargeKnob(this, "tubeunit", "airflow_knob");
    shared::bindSlider(this, airflow, patchCopy.airflow);

    vortex = shared::makeLargeKnob(this, "tubeunit", "vortex_knob");
    shared::bindSlider(this, vortex, patchCopy.vortex);

    width = shared::makeLargeKnob(this, "tubeunit", "width_knob");
    shared::bindSlider(this, width, patchCopy.width);

    center = shared::makeLargeKnob(this, "tubeunit", "center_knob");
    shared::bindSlider(this, center, patchCopy.center);

    decay = shared::makeLargeKnob(this, "tubeunit", "decay_knob");
    shared::bindSlider(this, decay, patchCopy.decay);

    angle = shared::makeLargeKnob(this, "tubeunit", "angle_knob");
    shared::bindSlider(this, angle, patchCopy.angle);

    root = shared::makeLargeKnob(this, "tubeunit", "root_knob");
    shared::bindSlider(this, root, patchCopy.root);

    spring = shared::makeLargeKnob(this, "tubeunit", "spring_knob");
    shared::bindSlider(this, spring, patchCopy.spring);

    outputLevel = shared::makeLargeKnob(this, "tubeunit", "level_knob");
    shared::bindSlider(this, outputLevel, patchCopy.outputLevel);

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
