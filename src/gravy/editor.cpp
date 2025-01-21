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

namespace sapphire_plugins::gravy
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

GravyEditor::GravyEditor(shared::audioToUIQueue_t &atou, shared::uiToAudioQueue_T &utoa,
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

    auto bg = shared::getSvgForPath("libs/sapphire/res/gravy.svg");
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

    const std::string modcode("gravy");

    airflow = shared::makeLargeKnob(this, modcode, "frequency_knob");
    shared::bindSlider(this, airflow, patchCopy.frequency);

    auto dim = shared::getPanelDimensions(modcode);
    setSize(dim.width, dim.height);
    resized();

    idleTimer = std::make_unique<shared::IdleTimer<GravyEditor>>(*this);
    idleTimer->startTimer(1000. / 60.);

    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, true});
}

GravyEditor::~GravyEditor()
{
    uiToAudio.push({shared::UIToAudioMsg::EDITOR_ATTACH_DETATCH, false});
    idleTimer->stopTimer();
}

void GravyEditor::resized()
{
    if (background)
    {
        background->setTransformToFit(getLocalBounds().toFloat(), juce::RectanglePlacement());
    }
}

void GravyEditor::idle() { shared::drainQueueFromUI(*this); }

} // namespace sapphire_plugins::gravy
