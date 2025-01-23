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

#ifndef SAPPHIRE_PLUGINS_SHARED_EDITOR_INTERACTIONS_H
#define SAPPHIRE_PLUGINS_SHARED_EDITOR_INTERACTIONS_H

#include "sapphire_panel.hpp"
#include "tooltip.h"

namespace sapphire_plugins::shared
{
template <typename Editor> struct IdleTimer : juce::Timer
{
    Editor &editor;
    IdleTimer(Editor &e) : editor(e) {}
    void timerCallback() override { editor.idle(); }
};

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

template <typename Ed> inline void drainQueueFromUI(Ed &editor)
{
    auto aum = editor.audioToUI.pop();
    while (aum.has_value())
    {
        switch (aum->action)
        {
        case AudioToUIMsg::UPDATE_PARAM:
        {
            auto pid = aum->paramId;
            auto val = aum->value;
            auto p = editor.patchCopy.paramMap.at(pid);
            if (p)
            {
                p->value = val;
                auto val01 = (val - p->meta.minVal) / (p->meta.maxVal - p->meta.minVal);
                auto sbi = editor.sliderByID.find(pid);
                if (sbi != editor.sliderByID.end() && sbi->second)
                {
                    sbi->second->setValue(val01, juce::dontSendNotification);
                }
            }
        }
        break;
        case AudioToUIMsg::UPDATE_VU:
            break;
        }
        aum = editor.audioToUI.pop();
    }
}

template <typename Processor>
void processUIQueueFromAudio(Processor *proc, const clap_output_events_t *outq)
{
    auto uiM = proc->uiToAudio.pop();
    while (uiM.has_value())
    {
        switch (uiM->action)
        {
        case shared::UIToAudioMsg::REQUEST_REFRESH:
        {
            // don't do it twice in one process obvs
            proc->pushFullUIRefresh();
        }
        break;
        case UIToAudioMsg::SET_PARAM:
        {
            auto dest = proc->patch.paramMap.at(uiM->paramId);

            dest->value = uiM->value;
            dest->lag.setTarget(uiM->value);

            clap_event_param_value_t p;
            p.header.size = sizeof(clap_event_param_value_t);
            p.header.time = 0;
            p.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            p.header.type = CLAP_EVENT_PARAM_VALUE;
            p.header.flags = 0;
            p.param_id = uiM->paramId;
            p.cookie = dest;

            p.note_id = -1;
            p.port_index = -1;
            p.channel = -1;
            p.key = -1;

            p.value = uiM->value;

            outq->try_push(outq, &p.header);
        }
        break;
        case UIToAudioMsg::BEGIN_EDIT:
        case UIToAudioMsg::END_EDIT:
        {
            clap_event_param_gesture_t p;
            p.header.size = sizeof(clap_event_param_gesture_t);
            p.header.time = 0;
            p.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            p.header.type = uiM->action == UIToAudioMsg::BEGIN_EDIT ? CLAP_EVENT_PARAM_GESTURE_BEGIN
                                                                    : CLAP_EVENT_PARAM_GESTURE_END;
            p.header.flags = 0;
            p.param_id = uiM->paramId;

            outq->try_push(outq, &p.header);
        }
        break;
        case UIToAudioMsg::EDITOR_ATTACH_DETATCH:
        {
            auto was = proc->isEditorAttached;

            proc->isEditorAttached = uiM->paramId;
            if (!was && proc->isEditorAttached)
            {
                proc->pushFullUIRefresh();
            }
        }
        break;
        }
        uiM = proc->uiToAudio.pop();
    }
}

template <typename Editor> inline void setTooltipValues(Editor *e, uint32_t id)
{
    auto &par = e->patchCopy.paramMap.at(id);
    e->tooltip->title = par->meta.name;
    auto oval = par->meta.valueToString(par->value);
    if (oval.has_value())
        e->tooltip->value = oval.value();
    else
        e->tooltip->value = "err";

    e->repaint();
}
template <typename Editor> inline void showTooltip(Editor *e, const juce::Slider &s, uint32_t id)
{
    if (!e->tooltip)
    {
        e->tooltip = std::make_unique<shared::Tooltip>();
        e->background->addChildComponent(*e->tooltip);
    }

    if (!s.getParentComponent())
        return;

    auto b = s.getBoundsInParent();
    auto pb = s.getParentComponent()->getLocalBounds();
    ;

    auto topHalf = true;
    if (b.getY() > pb.getHeight() / 2)
        topHalf = false;
    auto leftSide = true;
    if (b.getX() > pb.getWidth() / 2)
        leftSide = false;

    auto r = juce::Rectangle<int>(0, 0, 25, 11);
    if (topHalf)
        r = r.translated(0, b.getBottom());
    else
        r = r.translated(0, b.getY() - r.getHeight());

    if (leftSide)
        r = r.translated(b.getX(), 0);
    else
        r = r.translated(b.getRight() - r.getWidth(), 0);
    e->tooltip->setBounds(r);
    e->tooltip->setVisible(true);
    e->tooltip->toFront(true);
    setTooltipValues(e, id);
}

template <typename Editor> inline void updateTooltip(Editor *e, const juce::Slider &s, uint32_t id)
{
    if (e->tooltip)
        setTooltipValues(e, id);
}

template <typename Editor> inline void hideTooltip(Editor *e)
{
    if (e->tooltip)
        e->tooltip->setVisible(false);
}

template <typename Editor, typename Param>
inline void bindSlider(Editor *editor, const std::unique_ptr<juce::Slider> &slider, Param &p)
{
    slider->setTitle(p.meta.name);
    auto val01 = (p.value - p.meta.minVal) / (p.meta.maxVal - p.meta.minVal);
    auto dval01 = (p.meta.defaultVal - p.meta.minVal) / (p.meta.maxVal - p.meta.minVal);
    slider->setValue(val01, juce::dontSendNotification);
    slider->setDoubleClickReturnValue(true, dval01);

    slider->onDragStart =
        [editor, sl = juce::Component::SafePointer(slider.get()), id = p.meta.id]()
    {
        editor->uiToAudio.push({UIToAudioMsg::BEGIN_EDIT, id});
        if (sl)
            showTooltip(editor, *sl, id);
    };

    slider->onDragEnd = [editor, sl = juce::Component::SafePointer(slider.get()), id = p.meta.id]()
    {
        editor->uiToAudio.push({UIToAudioMsg::END_EDIT, id});
        if (sl)
            hideTooltip(editor);
    };

    slider->onValueChange = [editor, sl = juce::Component::SafePointer(slider.get()), par = p]()
    {
        if (!sl)
            return;
        float val = sl->getValue();
        // val is 0..1 so
        val = val * (par.meta.maxVal - par.meta.minVal) + par.meta.minVal;
        editor->uiToAudio.push({UIToAudioMsg::SET_PARAM, par.meta.id, val});
        editor->patchCopy.paramMap.at(par.meta.id)->value = val;

        updateTooltip(editor, *sl, par.meta.id);
        if (editor->flushOperator)
            editor->flushOperator();
    };

    editor->sliderByID[p.meta.id] = juce::Component::SafePointer<juce::Slider>(slider.get());
}

inline void set_control_position(juce::Component &control, float cx, float cy, float dx, float dy)
{
    juce::Point<float> real{cx + dx, cy + dy};
    juce::Point<int> rounded = real.toInt();
    control.setCentrePosition(rounded);
    control.setTransform(juce::AffineTransform::translation(real.getX() - rounded.getX(),
                                                            real.getY() - rounded.getY()));
}
template <typename Editor>
std::unique_ptr<juce::Slider> makeLargeKnob(Editor *editor, const std::string &prefix,
                                            const std::string pos)
{
    auto r = Sapphire::FindComponent(prefix, pos);
    auto cx = r.cx;
    auto cy = r.cy;

    static constexpr float dx = 1.5;
    static constexpr float dy = 0.0;
    auto kn = std::make_unique<juce::Slider>();
    kn->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kn->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    kn->setPopupMenuEnabled(true);
    kn->setSize(11, 11);
    kn->setRange(0, 1);
    kn->setValue(0.5);
    kn->setMouseDragSensitivity(100);
    kn->setDoubleClickReturnValue(true, 0.5);
    kn->setLookAndFeel(editor->lnf.get());

    editor->background->addAndMakeVisible(*kn);
    set_control_position(*kn, cx, cy, dx, dy);

    return kn;
}

template <typename Editor>
std::unique_ptr<juce::Slider> makeSlider(Editor *editor, const std::string &prefix,
                                         const std::string pos)
{
    auto r = Sapphire::FindComponent(prefix, pos);
    auto cx = r.cx;
    auto cy = r.cy;

    static constexpr float dx = 2;
    static constexpr float dy = 1;
    auto sl = std::make_unique<juce::Slider>();
    sl->setSliderStyle(juce::Slider::LinearVertical);
    sl->setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sl->setSize(10, 28);
    sl->setMouseDragSensitivity(28);
    sl->setRange(0, 1);
    sl->setValue(0.5);
    sl->setDoubleClickReturnValue(true, 0.5);
    sl->setLookAndFeel(editor->lnf.get());
    // TODO: Get the "snap to mouse position" to work with the scaling we have where we only use 90%
    // of the track (the remaining 10% is for the bottom part of the thumb; the thumb's "position"
    // is the very top pixel of the thumb). Until then, it doesn't work right throughout the whole
    // track, so we set this to false.
    sl->setSliderSnapsToMousePosition(false);

    editor->background->addAndMakeVisible(*sl);
    set_control_position(*sl, cx, cy, dx, dy);
    return sl;
}
} // namespace sapphire_plugins::shared

#endif // EDITOR_INTERACTIONS_H
