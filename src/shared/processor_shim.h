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

#ifndef SAPPHIRE_PLUGINS_SHARED_PROCESSOR_SHIM_H
#define SAPPHIRE_PLUGINS_SHARED_PROCESSOR_SHIM_H

#include "sst/plugininfra/version_information.h"
#include "sst/plugininfra/patch-support/patch_base_clap_adapter.h"
#include "sst/cpputils/ring_buffer.h"

#include "configuration.h"
#include <clap/clap.h>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>
#include <clapwrapper/vst3.h>

#include <memory>
#include "shared/editor_interactions.h"
#include "sst/clap_juce_shim/clap_juce_shim.h"

namespace sapphire_plugins::shared
{

static constexpr clap::helpers::MisbehaviourHandler misLevel =
    clap::helpers::MisbehaviourHandler::Ignore;
static constexpr clap::helpers::CheckingLevel checkLevel = clap::helpers::CheckingLevel::Maximal;

using plugHelper_t = clap::helpers::Plugin<misLevel, checkLevel>;

template <typename Processor>
struct ProcessorShim : plugHelper_t, sst::clap_juce_shim::EditorProvider
{
    static constexpr size_t smoothingBlock{8};
    static constexpr double smoothingMilis{5};

    ProcessorShim(const clap_plugin_descriptor_t *desc, const clap_host_t *host)
        : plugHelper_t(desc, host)
    {
        clapJuceShim = std::make_unique<sst::clap_juce_shim::ClapJuceShim>(this);
        clapJuceShim->setResizable(false);
    }

    Processor *asProcessor() { return static_cast<Processor *>(this); }
    const Processor *asProcessor() const { return static_cast<const Processor *>(this); }

    void pushFullUIRefresh()
    {
        SPLLOG("Full UI Refresh Requested");

        for (const auto *p : asProcessor()->patch.params)
        {
            shared::AudioToUIMsg au{shared::AudioToUIMsg::UPDATE_PARAM, p->meta.id, p->value};
            audioToUi.push(au);
        }
    }

    shared::audioToUIQueue_t audioToUi;
    shared::uiToAudioQueue_T uiToAudio;
    bool isEditorAttached{false};
    double sampleRate{0};

    uint32_t nextEventIndex{0};
    const clap_event_header_t *nextEvent{nullptr};
    uint32_t eventQSize{0};

    void startProcessEventTraversal(const clap_input_events_t *ev)
    {
        auto sz = ev->size(ev);

        eventQSize = sz;
        nextEventIndex = 0;
        if (sz != 0)
        {
            nextEvent = ev->get(ev, nextEventIndex);
        }
    }
    void processEventsUpTo(size_t sample, const clap_input_events_t *ev)
    {
        while (nextEvent && nextEvent->time <= sample)
        {
            handleEvent(nextEvent);
            nextEventIndex++;
            if (nextEventIndex < eventQSize)
                nextEvent = ev->get(ev, nextEventIndex);
            else
                nextEvent = nullptr;
        }
    }

    bool init() noexcept override { return true; }
    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        this->sampleRate = sampleRate;
        for (auto &[id, p] : asProcessor()->patch.paramMap)
            p->lag.setRateInMilliseconds(smoothingMilis, sampleRate, smoothingBlock);
        return true;
    }
    void deactivate() noexcept override {}

    bool implementsGui() const noexcept override { return clapJuceShim != nullptr; }
    std::unique_ptr<sst::clap_juce_shim::ClapJuceShim> clapJuceShim;
    ADD_SHIM_IMPLEMENTATION(clapJuceShim)
    ADD_SHIM_LINUX_TIMER(clapJuceShim)
    std::unique_ptr<juce::Component> createEditor() override
    {
        pushFullUIRefresh();
        auto res = std::make_unique<typename Processor::editor_t>(
            audioToUi, uiToAudio, [this]() { _host.paramsRequestFlush(); });
        // res->clapHost = _host.host();

        return res;
    }

    bool registerOrUnregisterTimer(clap_id &id, int ms, bool reg) override
    {
        if (!_host.canUseTimerSupport())
            return false;
        if (reg)
        {
            _host.timerSupportRegister(ms, &id);
        }
        else
        {
            _host.timerSupportUnregister(id);
        }
        return true;
    }

    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override { return asProcessor()->patch.params.size(); }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsInfo(paramIndex, info,
                                                                asProcessor()->patch);
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsValue(paramId, value,
                                                                 asProcessor()->patch);
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsValueToText(paramId, value, display,
                                                                       size, asProcessor()->patch);
    }
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsTextToValue(paramId, display, value,
                                                                       asProcessor()->patch);
    }

    void paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept override
    {
        auto sz = in->size(in);

        for (int i = 0; i < sz; ++i)
        {
            const clap_event_header_t *nextEvent{nullptr};
            nextEvent = in->get(in, i);
            handleEvent(nextEvent);
        }

        shared::processUIQueueFromAudio(asProcessor(), out);
    }

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override
    {
        if (isInput)
            if (Processor::hasStereoInput)
                return 1;
        if (!isInput)
            if (Processor::hasStereoOutput)
                return 1;

        return 0;
    }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        if (index != 0)
            return false;
        if (isInput && !Processor::hasStereoInput)
            return false;
        if (!isInput && !Processor::hasStereoOutput)
            return false;

        info->id = isInput ? 2112 : 90125;
        if (Processor::hasStereoInput && Processor::hasStereoOutput)
            info->in_place_pair = isInput ? 90125 : 2112;

        if (isInput)
            strncpy(info->name, "Main Input", sizeof(info->name));
        else
            strncpy(info->name, "Main Out", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        return true;
    }

    bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream *ostream) noexcept override
    {
        // engine->prepForStream();
        return sst::plugininfra::patch_support::patchToOutStream(asProcessor()->patch, ostream);
    }
    bool stateLoad(const clap_istream *istream) noexcept override
    {
        if (!sst::plugininfra::patch_support::inStreamToPatch(istream, asProcessor()->patch))
            return false;

        for (auto &[id, p] : asProcessor()->patch.paramMap)
        {
            p->snap();
        }

        // engine->postLoad();
        _host.paramsRequestFlush();
        return true;
    }

    bool handleEvent(const clap_event_header_t *nextEvent)
    {
        auto res = true;
        if (nextEvent->space_id == CLAP_CORE_EVENT_SPACE_ID)
        {
            switch (nextEvent->type)
            {
            case CLAP_EVENT_PARAM_VALUE:
            {
                auto pevt = reinterpret_cast<const clap_event_param_value_t *>(nextEvent);
                auto par =
                    sst::plugininfra::patch_support::paramFromClapEvent<typename Processor::param_t,
                                                                        clap_event_param_value_t>(
                        pevt, asProcessor()->patch);
                if (par)
                {
                    par->value = pevt->value;
                    par->lag.setTarget(pevt->value);

                    shared::AudioToUIMsg au{shared::AudioToUIMsg::UPDATE_PARAM, par->meta.id,
                                            par->value};
                    audioToUi.push(au);
                }
            }
            break;
            default:
                res = asProcessor()->handleNonParamEvent(nextEvent);
                break;
            }
        }
        return res;
    }
};
} // namespace sapphire_plugins::shared

#endif // PROCESSOR_SHIM_H
