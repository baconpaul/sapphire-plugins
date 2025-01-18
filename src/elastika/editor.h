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

#ifndef SAPPHIRE_PLUGINS_ELASTIKA_EDITOR_H
#define SAPPHIRE_PLUGINS_ELASTIKA_EDITOR_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "sst/cpputils/ring_buffer.h"
#include "shared/sapphire_lnf.h"

#include "patch.h"
#include "shared/editor_interactions.h"

namespace sapphire_plugins::elastika
{
struct ElastikaEditor : public juce::Component
{
    Patch patchCopy;

    ElastikaEditor(shared::audioToUIQueue_t &atou, shared::uiToAudioQueue_T &utoa,
                   std::function<void()> flushOperator);
    ~ElastikaEditor();

    void bindSlider(const std::unique_ptr<juce::Slider> &slider, Param &p);
    std::unordered_map<uint32_t, juce::Component::SafePointer<juce::Slider>> sliderByID;

    void resized() override;

    void idle();
    std::unique_ptr<juce::Timer> idleTimer;

    std::unique_ptr<juce::Drawable> background;
    std::unique_ptr<juce::Slider> input_tilt_knob;
    std::unique_ptr<juce::Slider> output_tilt_knob;
    std::unique_ptr<juce::Slider> drive_knob;
    std::unique_ptr<juce::Slider> level_knob;

    std::unique_ptr<juce::Slider> fric_slider;
    std::unique_ptr<juce::Slider> mass_slider;
    std::unique_ptr<juce::Slider> stif_slider;
    std::unique_ptr<juce::Slider> span_slider;
    std::unique_ptr<juce::Slider> curl_slider;

    std::unique_ptr<shared::LookAndFeel> lnf;

    shared::audioToUIQueue_t &audioToUI;
    shared::uiToAudioQueue_T &uiToAudio;
    std::function<void()> flushOperator;
};
} // namespace sapphire_plugins::elastika
#endif
