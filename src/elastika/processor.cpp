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
    ElastikaClap(const clap_host *h) : plugHelper_t(getDescriptor(), h) {}

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