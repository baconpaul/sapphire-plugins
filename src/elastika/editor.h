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

namespace sapphire_plugins::elastika
{

struct AudioToUIMsg
{
    enum Action : uint32_t
    {
        UPDATE_PARAM,
        UPDATE_VU,
    } action;
    uint32_t paramId{0};
    float value{0}, value2{0};
};
struct UIToAudioMsg
{
    enum Action : uint32_t
    {
        REQUEST_REFRESH,
        SET_PARAM,
        BEGIN_EDIT,
        END_EDIT,
        EDITOR_ATTACH_DETATCH, // paramid is true for attach and false for detach
    } action;
    uint32_t paramId{0};
    float value{0};
};
using audioToUIQueue_t = sst::cpputils::SimpleRingBuffer<AudioToUIMsg, 1024 * 16>;
using uiToAudioQueue_T = sst::cpputils::SimpleRingBuffer<UIToAudioMsg, 1024 * 64>;

struct ElastikaEditor : public juce::Component
{
    Patch patchCopy;

    ElastikaEditor(audioToUIQueue_t &atou, uiToAudioQueue_T &utoa,
                   std::function<void()> flushOperator);
    ~ElastikaEditor();

    void bindSlider(const std::unique_ptr<juce::Slider> &slider, Param &p);
    std::unordered_map<uint32_t, juce::Component::SafePointer<juce::Slider>> sliderByID;

    void resized() override;

    void idle();
    std::unique_ptr<juce::Timer> idleTimer;

    std::unique_ptr<juce::Slider> make_large_knob(const std::string &pos);
    std::unique_ptr<juce::Slider> make_slider(const std::string &pos);

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

    audioToUIQueue_t &audioToUI;
    uiToAudioQueue_T &uiToAudio;
    std::function<void()> flushOperator;
};
} // namespace sapphire_plugins::elastika
#endif
