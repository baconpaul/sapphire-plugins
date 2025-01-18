/*
 * Sapphire Plugins
 *
 * Bringing the magical world of CosineKitty's sapphire plugins for rack to your DAW
 *
 * Copyright 2024-2025, Don Cross, Paul Walker and Various authors, as described in the github
 * transaction log.
 *
 * The source code and license are at https://github.com/baconpaul/sapphire-plugins
 */

#include "sst/plugininfra/version_information.h"

#include "configuration.h"
#include <clap/clap.h>

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hxx>
#include <clapwrapper/vst3.h>

#include <memory>
#include "sst/clap_juce_shim/clap_juce_shim.h"

#include "elastika_engine.hpp"
#include "elastika.h"
#include "patch.h"
#include "editor.h"

namespace sapphire_plugins::elastika
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
        "Elastika",
        "Sapphirel",
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
    std::unique_ptr<Sapphire::ElastikaEngine> engine;
    Patch patch;
    static constexpr size_t smoothingBlock{8};
    static constexpr double smoothingMilis{5};
    size_t blockPos{0};

    ElastikaClap(const clap_host *h) : plugHelper_t(getDescriptor(), h)
    {
        engine = std::make_unique<Sapphire::ElastikaEngine>();
    }

    double sampleRate{0};

  protected:
    bool init() noexcept override { return true; }
    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        this->sampleRate = sampleRate;
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
                engine->setFriction(patch.friction.lag.v);
                engine->setStiffness(patch.stiffness.lag.v);
                engine->setSpan(patch.span.lag.v);
                engine->setCurl(patch.curl.lag.v);
                engine->setMass(patch.mass.lag.v);
                engine->setDrive(patch.drive.lag.v);
                engine->setGain(patch.level.lag.v);
                engine->setInputTilt(patch.inputTilt.lag.v);
                engine->setOutputTilt(patch.outputTilt.lag.v);
            }
            engine->process(sampleRate, in[0][s], in[1][s], out[0][s], out[1][s]);
            blockPos = (blockPos + 1) & (smoothingBlock - 1);
        }

        return CLAP_PROCESS_CONTINUE;
    }
    void reset() noexcept override { engine->quiet(); }

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
        auto ss = patch.toState();
        auto c = ss.c_str();
        auto s = ss.length() + 1; // write the null terminator
        while (s > 0)
        {
            auto r = ostream->write(ostream, c, s);
            if (r < 0)
                return false;
            s -= r;
            c += r;
        }
        return true;
    }
    bool stateLoad(const clap_istream *istream) noexcept override
    {
        static constexpr uint32_t initSize = 1 << 16, chunkSize = 1 << 8;
        std::vector<char> buffer;
        buffer.resize(initSize);

        int64_t rd{0};
        int64_t totalRd{0};
        auto bp = buffer.data();

        while ((rd = istream->read(istream, bp, chunkSize)) > 0)
        {
            bp += rd;
            totalRd += rd;
            if (totalRd >= buffer.size() - chunkSize - 1)
            {
                buffer.resize(buffer.size() * 2);
                bp = buffer.data() + totalRd;
            }
        }
        buffer[totalRd] = 0;

        if (totalRd == 0)
        {
            SPLLOG("Received stream size 0. Invalid state");
            return false;
        }

        auto data = std::string(buffer.data());
        patch.fromState(data);

        for (auto &[id, p] : patch.paramMap)
            p->snap();

        // engine->postLoad();
        _host.paramsRequestFlush();
        return true;
    }

    bool implementsParams() const noexcept override { return true; }
    uint32_t paramsCount() const noexcept override { return patch.params.size(); }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override
    {
        auto *param = patch.params[paramIndex];
        auto &md = param->meta;
        md.toClapParamInfo<CLAP_NAME_SIZE, clap_param_info_t>(info);
        info->cookie = (void *)param;
        return true;
    }
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        *value = patch.paramMap[paramId]->value;
        return true;
    }
    bool paramsValueToText(clap_id paramId, double value, char *display,
                           uint32_t size) noexcept override
    {
        auto it = patch.paramMap.find(paramId);
        if (it == patch.paramMap.end())
        {
            return false;
        }
        auto valdisp = it->second->meta.valueToString(value);
        if (!valdisp.has_value())
            return false;

        strncpy(display, valdisp->c_str(), size);
        display[size - 1] = 0;
        return true;
    }
    bool paramsTextToValue(clap_id paramId, const char *display, double *value) noexcept override
    {
        auto it = patch.paramMap.find(paramId);
        if (it == patch.paramMap.end())
        {
            return false;
        }
        std::string err;
        auto val = it->second->meta.valueFromString(display, err);
        if (!val.has_value())
        {
            return false;
        }
        *value = *val;
        return true;
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

        // engine->processUIQueue(out);
    }

    bool handleEvent(const clap_event_header_t *nextEvent)
    {
        if (nextEvent->space_id == CLAP_CORE_EVENT_SPACE_ID)
        {
            switch (nextEvent->type)
            {
            case CLAP_EVENT_PARAM_VALUE:
            {
                auto pevt = reinterpret_cast<const clap_event_param_value *>(nextEvent);
                auto pid = pevt->param_id;
                ;
                auto p = patch.paramMap.at(pid);

                p->value = pevt->value;
                p->lag.setTarget(pevt->value);

                // AudioToUIMsg au = {AudioToUIMsg::UPDATE_PARAM, pid, value};
                // audioToUi.push(au);
            }
            break;
            default:
                break;
            }
        }
        return true;
    }

  public:
    bool implementsGui() const noexcept override
    {
        return false;
    } // return clapJuceShim != nullptr; }
    std::unique_ptr<sst::clap_juce_shim::ClapJuceShim> clapJuceShim;
    ADD_SHIM_IMPLEMENTATION(clapJuceShim)
    ADD_SHIM_LINUX_TIMER(clapJuceShim)
    std::unique_ptr<juce::Component> createEditor() override
    {
        /* auto res = std::make_unique<baconpaul::six_sines::ui::SixSinesEditor>(
            engine->audioToUi, engine->uiToAudio, [this]() { _host.paramsRequestFlush(); });
        res->clapHost = _host.host();
        return res; */
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

} // namespace sapphire_plugins::elastika