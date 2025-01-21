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

#include "elastika_engine.hpp"
#include "elastika.h"
#include "patch.h"
#include "editor.h"

namespace sapphire_plugins::elastika
{

const clap_plugin_descriptor *getDescriptor()
{
    static const char *features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_FILTER,
                                     nullptr};

    static clap_plugin_descriptor desc = {
        CLAP_VERSION,
        pluginId,
        "Elastika",
        "Sapphire",
        "",
        "",
        "",
        sst::plugininfra::VersionInformation::project_version_and_hash,
        "Physical modelling with masses and springs",
        &features[0]};
    return &desc;
}

struct ElastikaClap : public shared::ProcessorShim<ElastikaClap>
{
    using editor_t = ElastikaEditor;
    using patch_t = Patch;
    using param_t = Param;
    static constexpr bool hasStereoInput{true};
    static constexpr bool hasStereoOutput{true};

    std::unique_ptr<Sapphire::ElastikaEngine> engine;
    Patch patch;
    size_t blockPos{0};

    ElastikaClap(const clap_host *h) : shared::ProcessorShim<ElastikaClap>(getDescriptor(), h)
    {
        engine = std::make_unique<Sapphire::ElastikaEngine>();
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

        processEventsUpTo(process->frames_count, ev);
        return CLAP_PROCESS_CONTINUE;
    }
    void reset() noexcept override { engine->quiet(); }
    bool handleNonParamEvent(const clap_event_header_t *nextEvent) { return false; }
};

const clap_plugin *makePlugin(const clap_host *h)
{
    auto res = new ElastikaClap(h);
    return res->clapPlugin();
}

} // namespace sapphire_plugins::elastika