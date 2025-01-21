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

#ifndef SAPPHIRE_PLUGINS_GALAXY_PATCH_H
#define SAPPHIRE_PLUGINS_GALAXY_PATCH_H

#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <clap/clap.h>
#include "configuration.h"
#include "galaxy.h"
#include "sst/plugininfra/patch-support/patch_base.h"
#include "shared/param_with_lag.h"

namespace sapphire_plugins::galaxy
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

    Param replace, brightness, detune, bigness, mix;

    Patch()
        : pats::PatchBase<Patch, Param>(),
          replace(floatMd().withName("Replace").withID(100).asPercent().withDefault(0.5)),
          brightness(floatMd().withName("Brightness").withID(110).asPercent().withDefault(0.5)),
          detune(floatMd().withName("Detune").withID(120).asPercent().withDefault(0.5)),
          bigness(floatMd().withName("Bigness").withID(130).asPercent().withDefault(0.5)),
          mix(floatMd().withName("Mix").withID(140).asPercent().withDefault(0.5))
    {
        this->pushSingleParam(&replace);
        this->pushSingleParam(&brightness);
        this->pushSingleParam(&detune);
        this->pushSingleParam(&bigness);
        this->pushSingleParam(&mix);

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
} // namespace sapphire_plugins::galaxy

#endif
