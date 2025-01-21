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

#include "shared/processor_shim.h"

#include "tubeunit_engine.hpp"
#include "tube_unit.h"
#include "patch.h"
#include "editor.h"

namespace sapphire_plugins::tube_unit
{

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
        "Physical modelling at the end of a tube",
        &features[0]};
    return &desc;
}

struct TubeUnitClap : public shared::ProcessorShim<TubeUnitClap>
{
    using editor_t = TubeUnitEditor;
    using patch_t = Patch;
    using param_t = Param;
    static constexpr bool hasStereoInput{true};
    static constexpr bool hasStereoOutput{true};

    std::unique_ptr<Sapphire::TubeUnitEngine> engine;
    Patch patch;
    size_t blockPos{0};

    TubeUnitClap(const clap_host *h) : shared::ProcessorShim<TubeUnitClap>(getDescriptor(), h)
    {
        engine = std::make_unique<Sapphire::TubeUnitEngine>();
    }

    bool activate(double sampleRate, uint32_t minFrameCount,
                  uint32_t maxFrameCount) noexcept override
    {
        auto res =
            shared::ProcessorShim<TubeUnitClap>::activate(sampleRate, minFrameCount, maxFrameCount);
        engine->setSampleRate(sampleRate);
        return res;
    }

    clap_process_status process(const clap_process *process) noexcept override
    {
        auto ev = process->in_events;
        auto outq = process->out_events;

        float **in = process->audio_inputs[0].data32;
        float **out = process->audio_outputs[0].data32;

        shared::processUIQueueFromAudio(this, outq);

        startProcessEventTraversal(ev);

        for (auto s = 0U; s < process->frames_count; ++s)
        {
            if (blockPos == 0)
            {
                processEventsUpTo(s, ev);

                for (auto &[i, p] : patch.paramMap)
                    p->lag.process();
                engine->setGain(patch.outputLevel.lag.v);
                engine->setAirflow(patch.airflow.lag.v);
                engine->setVortex(patch.vortex.lag.v);
                engine->setBypassWidth(patch.width.lag.v);
                engine->setBypassCenter(patch.center.lag.v);
                engine->setReflectionAngle(M_PI * patch.angle.lag.v);
                engine->setReflectionDecay(patch.decay.lag.v);
                engine->setRootFrequency(4 * std::pow(2.f, patch.root.lag.v));
                engine->setSpringConstant(0.005f * std::pow(10.0f, 4.0f * patch.spring.lag.v));
            }
            engine->process(out[0][s], out[1][s], in[0][s], in[1][s]);
            blockPos = (blockPos + 1) & (smoothingBlock - 1);
        }

        processEventsUpTo(process->frames_count, ev);
        return CLAP_PROCESS_CONTINUE;
    }
    void reset() noexcept override
    {
        engine->setQuiet(true);
        engine->setQuiet(false);
    }

    bool handleNonParamEvent(const clap_event_header_t *nextEvent) { return false; }
};

const clap_plugin *makePlugin(const clap_host *h)
{
    auto res = new TubeUnitClap(h);
    return res->clapPlugin();
}

} // namespace sapphire_plugins::tube_unit
