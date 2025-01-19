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

    auto bg = shared::getSvgForPath("libs/sapphire/export/elastika.svg");
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

    input_tilt_knob = shared::makeLargeKnob(this, "elastika", "input_tilt_knob");
    shared::bindSlider(this, input_tilt_knob, patchCopy.inputTilt);

    output_tilt_knob = shared::makeLargeKnob(this, "elastika", "output_tilt_knob");
    shared::bindSlider(this, output_tilt_knob, patchCopy.outputTilt);

    drive_knob = shared::makeLargeKnob(this, "elastika", "drive_knob");
    shared::bindSlider(this, drive_knob, patchCopy.drive);

    level_knob = shared::makeLargeKnob(this, "elastika", "level_knob");
    shared::bindSlider(this, level_knob, patchCopy.level);

    fric_slider = shared::makeSlider(this, "elastika", "fric_slider");
    shared::bindSlider(this, fric_slider, patchCopy.friction);

    curl_slider = shared::makeSlider(this, "elastika", "curl_slider");
    shared::bindSlider(this, curl_slider, patchCopy.curl);

    span_slider = shared::makeSlider(this, "elastika", "span_slider");
    shared::bindSlider(this, span_slider, patchCopy.span);

    mass_slider = shared::makeSlider(this, "elastika", "mass_slider");
    shared::bindSlider(this, mass_slider, patchCopy.mass);

    stif_slider = shared::makeSlider(this, "elastika", "stif_slider");
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

} // namespace sapphire_plugins::elastika