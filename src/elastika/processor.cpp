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
        "org.sapphire_plugin.elastika",
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
        return true;
    }
    void deactivate() noexcept override {}
    clap_process_status process(const clap_process *process) noexcept override
    {
        float **in = process->audio_inputs[0].data32;
        float **out = process->audio_outputs[0].data32;

        for (auto s = 0U; s < process->frames_count; ++s)
        {
            engine->process(sampleRate, in[0][s], in[1][s], out[0][s], out[1][s]);
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