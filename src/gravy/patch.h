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

#ifndef SAPPHIRE_PLUGINS_GRAVY_PATCH_H
#define SAPPHIRE_PLUGINS_GRAVY_PATCH_H

#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <clap/clap.h>
#include "configuration.h"
#include "gravy.h"
#include "sst/plugininfra/patch-support/patch_base.h"
#include "shared/param_with_lag.h"

namespace sapphire_plugins::gravy
{
namespace scpu = sst::cpputils;
namespace pats = sst::plugininfra::patch_support;
using md_t = sst::basic_blocks::params::ParamMetaData;

using Param = shared::ParamWithLag;

struct Patch : pats::PatchBase<Patch, Param>
{
    static constexpr uint32_t patchVersion{1};
    static constexpr const char *id{pluginId};
    char name[256]{""};

    static constexpr uint32_t floatFlags{CLAP_PARAM_IS_AUTOMATABLE};

    static md_t floatMd() { return md_t().asFloat().withFlags(floatFlags); }
    static md_t intMd() { return md_t().asInt().withFlags(floatFlags | CLAP_PARAM_IS_STEPPED); }

    Param frequency, resonance, mix, gain, mode;

    Patch()
        : pats::PatchBase<Patch, Param>(), frequency(floatMd()
                                                         .withName("Frequency")
                                                         .withID(100)
                                                         .withRange(-5, 5)
                                                         .withDefault(0)
                                                         .withLinearScaleFormatting("")),
          resonance(
              floatMd().withName("Resonance").withID(110).asPercent().withDefault(sqrt(2) / 2.0)),
          mix(floatMd().withName("Mix").withID(120).asPercent().withDefault(1)),
          gain(floatMd().withName("Gain").withID(130).asPercent().withDefault(0.5)),
          mode(intMd()
                   .withName("Mode")
                   .withID(140)
                   .withRange(0, 2)
                   .withDefault(1)
                   .withUnorderedMapFormatting({{0, "LowPass"}, {1, "BandPass"}, {2, "HighPass"}}))

    {
        this->pushSingleParam(&frequency);
        this->pushSingleParam(&resonance);
        this->pushSingleParam(&mix);
        this->pushSingleParam(&gain);
        this->pushSingleParam(&mode);

        onResetToInit = [](auto &patch)
        {
            for (auto &[i, p] : patch.paramMap)
            {
                p->snap();
            }
        };

        resetToInit();
    }

    void migratePatchFromVersion(uint32_t) {}
    float migrateParamValueFromVersion(Param *, float v, uint32_t) { return v; }
};
} // namespace sapphire_plugins::gravy

#endif
