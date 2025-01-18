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

struct JNK : public juce::Component
{
    void paint(juce::Graphics &g) override
    {
        SPLLOG("paint " << getLocalBounds().toString());
        g.fillAll(juce::Colours::red);
    }
};

ElastikaEditor::ElastikaEditor(audioToUIQueue_t &atou, uiToAudioQueue_T &utoa,
                               std::function<void()> flushOperator)
{

    auto knob_xml = juce::XmlDocument::parse(*shared::getSvgForPath("res/knob_graphics/knob.svg"));
    auto marker_xml = juce::XmlDocument::parse(*shared::getSvgForPath("res/knob_graphics/knob-marker.svg"));
    lnf = std::make_unique<shared::LookAndFeel>(juce::Drawable::createFromSVG(*knob_xml),
                                                  juce::Drawable::createFromSVG(*marker_xml));
    setLookAndFeel(lnf.get());

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

    auto r = Sapphire::FindComponent("elastika", "output_tilt_knob");
    auto cx = r.cx;
    auto cy = r.cy;

    junk = std::make_unique<JNK>();
    junk->setSize(11,11);
    background->addAndMakeVisible(*junk);
    set_control_position(*junk, cx, cy, 0.5, 0.5);


    setSize(300, 600);
    resized();
}

void ElastikaEditor::resized()
{
    if (background)
    {
        background->setTransformToFit(getLocalBounds().toFloat(), juce::RectanglePlacement());
    }
}


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

    background->addAndMakeVisible(*kn);
    set_control_position(*kn, cx, cy, dx, dy);

    return kn;
}


} // namespace sapphire_plugins::elastika