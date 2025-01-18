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
#include "sst/clap_juce_shim/clap_juce_shim.h"

#include "tubeunit_engine.hpp"
#include "tube_unit.h"
#include "patch.h"
#include "editor.h"

namespace sapphire_plugins::tube_unit
{

static constexpr clap::helpers::MisbehaviourHandler misLevel =
    clap::helpers::MisbehaviourHandler::Ignore;
static constexpr clap::helpers::CheckingLevel checkLevel = clap::helpers::CheckingLevel::Maximal;

using plugHelper_t = clap::helpers::Plugin<misLevel, checkLevel>;

const clap_plugin_descriptor *getDescriptor()
{
    static const char *features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_FILTER,
                                     nullptr};

    static clap_plugin_descriptor desc = {
        CLAP_VERSION,
        pluginId,
        "Tube Unit",
        "Sapphire",
        "",
        "",
        "",
        sst::plugininfra::VersionInformation::project_version_and_hash,
        "Fun with Physical Modelling",
        &features[0]};
    return &desc;
}

struct ElastikaClap : public plugHelper_t, sst::clap_juce_shim::EditorProvider
{
    std::unique_ptr<Sapphire::TubeUnitEngine> engine;
    Patch patch;
    static constexpr size_t smoothingBlock{8};
    static constexpr double smoothingMilis{5};
    size_t blockPos{0};

    ElastikaClap(const clap_host *h) : plugHelper_t(getDescriptor(), h)
    {
        engine = std::make_unique<Sapphire::TubeUnitEngine>();

        // clapJuceShim = std::make_unique<sst::clap_juce_shim::ClapJuceShim>(this);
        // clapJuceShim->setResizable(false);
    }

    double sampleRate{0};

    shared::audioToUIQueue_t audioToUi;
    shared::uiToAudioQueue_T uiToAudio;
    bool isEditorAttached{false};

  protected:
    bool init() noexcept override { return true; }
    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        this->sampleRate = sampleRate;
        engine->setSampleRate(sampleRate);
        for (auto &[id, p] : patch.paramMap)
            p->lag.setRateInMilliseconds(smoothingMilis, sampleRate, smoothingBlock);
        return true;
    }
    void deactivate() noexcept override {}
    clap_process_status process(const clap_process *process) noexcept override
    {
        auto ev = process->in_events;
        auto outq = process->out_events;
        auto sz = ev->size(ev);

        float **in = process->audio_inputs[0].data32;
        float **out = process->audio_outputs[0].data32;

        shared::processUIQueueFromAudio(this, outq);

        const clap_event_header_t *nextEvent{nullptr};
        uint32_t nextEventIndex{0};
        if (sz != 0)
        {
            nextEvent = ev->get(ev, nextEventIndex);
        }

        for (auto s = 0U; s < process->frames_count; ++s)
        {
            while (nextEvent && nextEvent->time <= s)
            {
                handleEvent(nextEvent);
                nextEventIndex++;
                if (nextEventIndex < sz)
                    nextEvent = ev->get(ev, nextEventIndex);
                else
                    nextEvent = nullptr;
            }

            if (blockPos == 0)
            {
                for (auto &[i, p] : patch.paramMap)
                    p->lag.process();
                engine->setAirflow(patch.airflow.lag.v);
                engine->setVortex(patch.vortex.lag.v);
                engine->setBypassWidth(patch.bypassWidth.lag.v);
                engine->setBypassCenter(patch.bypassCenter.lag.v);
                engine->setReflectionAngle(M_PI * patch.reflectionAngle.lag.v);
                engine->setReflectionDecay(patch.reflectionDecay.lag.v);
                engine->setRootFrequency(4 * std::pow(2.f, patch.rootFrequency.lag.v));
                engine->setSpringConstant(0.005f * std::pow(10.0f, 4.0f * patch.stiffness.lag.v));
            }
            engine->process(out[0][s], out[1][s], in[0][s], in[1][s]);
            blockPos = (blockPos + 1) & (smoothingBlock - 1);
        }

        return CLAP_PROCESS_CONTINUE;
    }
    void reset() noexcept override
    {
        engine->setQuiet(true);
        engine->setQuiet(false);
    }

    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override { return 1; }
    bool audioPortsInfo(uint32_t index, bool isInput,
                        clap_audio_port_info *info) const noexcept override
    {
        if (index != 0)
            return false;
        info->id = isInput ? 2112 : 90125;
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
        return sst::plugininfra::patch_support::patchToOutStream(patch, ostream);
    }
    bool stateLoad(const clap_istream *istream) noexcept override
    {
        if (!sst::plugininfra::patch_support::inStreamToPatch(istream, patch))
            return false;

        for (auto &[id, p] : patch.paramMap)
        {
            p->snap();
        }

        // engine->postLoad();
        _host.paramsRequestFlush();
        return true;
    }

    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override { return patch.params.size(); }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsInfo(paramIndex, info, patch);
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsValue(paramId, value, patch);
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsValueToText(paramId, value, display,
                                                                       size, patch);
    }
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override
    {
        return sst::plugininfra::patch_support::patchParamsTextToValue(paramId, display, value,
                                                                       patch);
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

        shared::processUIQueueFromAudio(this, out);
    }

    bool handleEvent(const clap_event_header_t *nextEvent)
    {
        if (nextEvent->space_id == CLAP_CORE_EVENT_SPACE_ID)
        {
            switch (nextEvent->type)
            {
            case CLAP_EVENT_PARAM_VALUE:
            {
                auto pevt = reinterpret_cast<const clap_event_param_value_t *>(nextEvent);
                auto par = sst::plugininfra::patch_support::paramFromClapEvent<
                    Param, clap_event_param_value_t>(pevt, patch);
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
                break;
            }
        }
        return true;
    }

  public:
    void pushFullUIRefresh()
    {
        SPLLOG("Full UI Refresh Requested");

        for (const auto *p : patch.params)
        {
            shared::AudioToUIMsg au{shared::AudioToUIMsg::UPDATE_PARAM, p->meta.id, p->value};
            audioToUi.push(au);
        }
    }

    bool implementsGui() const noexcept override { return false; /*clapJuceShim != nullptr;*/ }
    std::unique_ptr<sst::clap_juce_shim::ClapJuceShim> clapJuceShim;
    ADD_SHIM_IMPLEMENTATION(clapJuceShim)
    ADD_SHIM_LINUX_TIMER(clapJuceShim)
    std::unique_ptr<juce::Component> createEditor() override
    {
        // pushFullUIRefresh();
        // auto res = std::make_unique<ElastikaEditor>(audioToUi, uiToAudio,
        //                                                    [this]() { _host.paramsRequestFlush();
        //                                                    });
        // res->clapHost = _host.host();

        return nullptr;
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
};

const clap_plugin *makePlugin(const clap_host *h)
{
    auto res = new ElastikaClap(h);
    return res->clapPlugin();
}

} // namespace sapphire_plugins::tube_unit
