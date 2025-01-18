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
#ifndef SAPPHIRE_PLUGINS_SHARED_SAPPHIRE_LNF_H
#define SAPPHIRE_PLUGINS_SHARED_SAPPHIRE_LNF_H

#include <memory>
#include "juce_gui_basics/juce_gui_basics.h"

namespace sapphire_plugin::shared
{
class LookAndFeel : public juce::LookAndFeel_V4
{
  public:
    LookAndFeel(std::unique_ptr<juce::Drawable> knob, std::unique_ptr<juce::Drawable> marker);

    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                          float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider &slider) override;

    void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider &slider) override;

    juce::Slider::SliderLayout getSliderLayout(juce::Slider &slider) override;

  private:
    std::unique_ptr<juce::Drawable> knob_;
    std::unique_ptr<juce::Drawable> knob_marker_;
    std::unique_ptr<juce::Image> knob_cache_;

    int rotary_scale_factor_;
};
} // namespace sapphire_plugin::shared

#endif // SAPPHIRE_PLUGINS_SHARED_SAPPHIRE_LNF_H
