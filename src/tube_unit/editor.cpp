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

    auto bg = shared::getSvgForPath("libs/sapphire/export/tubeunit.svg");
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

    const std::string modcode("tubeunit_export");

    airflow = shared::makeLargeKnob(this, modcode, "airflow_knob");
    shared::bindSlider(this, airflow, patchCopy.airflow);

    vortex = shared::makeLargeKnob(this, modcode, "vortex_knob");
    shared::bindSlider(this, vortex, patchCopy.vortex);

    width = shared::makeLargeKnob(this, modcode, "width_knob");
    shared::bindSlider(this, width, patchCopy.width);

    center = shared::makeLargeKnob(this, modcode, "center_knob");
    shared::bindSlider(this, center, patchCopy.center);

    decay = shared::makeLargeKnob(this, modcode, "decay_knob");
    shared::bindSlider(this, decay, patchCopy.decay);

    angle = shared::makeLargeKnob(this, modcode, "angle_knob");
    shared::bindSlider(this, angle, patchCopy.angle);

    root = shared::makeLargeKnob(this, modcode, "root_knob");
    shared::bindSlider(this, root, patchCopy.root);

    spring = shared::makeLargeKnob(this, modcode, "spring_knob");
    shared::bindSlider(this, spring, patchCopy.spring);

    mix = shared::makeLargeKnob(this, modcode, "mix_knob");
    shared::bindSlider(this, mix, patchCopy.mix);

    auto dim = shared::getPanelDimensions(modcode);
    setSize(dim.width, dim.height);
    resized();

    idleTimer = std::make_unique<shared::IdleTimer<TubeUnitEditor>>(*this);
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
