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

#ifndef SAPPHIRE_PLUGINS_TUBE_UNIT_PATCH_H
#define SAPPHIRE_PLUGINS_TUBE_UNIT_PATCH_H

#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <clap/clap.h>
#include "configuration.h"
#include "tube_unit.h"
#include "sst/plugininfra/patch-support/patch_base.h"
#include "shared/param_with_lag.h"

namespace sapphire_plugins::tube_unit
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

    Param airflow;
    Param vortex;
    Param width;
    Param center;
    Param decay;
    Param angle;
    Param root;
    Param spring;
    Param mix;

    Patch()
        : pats::PatchBase<Patch, Param>(), airflow(floatMd()
                                                       .withName("Airflow")
                                                       .withID(100)
                                                       .withRange(0, 5)
                                                       .withDefault(1)
                                                       .withLinearScaleFormatting("")),
          vortex(floatMd().withName("Vortex").withID(110).asPercent().withDefault(0.5)),
          width(floatMd()
                    .withName("Bypass Width")
                    .withID(120)
                    .withRange(0.5, 20)
                    .withDefault(6)
                    .withLinearScaleFormatting("")),
          center(floatMd()
                     .withName("Bypass Center")
                     .withID(130)
                     .withRange(-10, 10)
                     .withDefault(0.f)
                     .withLinearScaleFormatting("")),
          decay(floatMd()
                    .withName("Reflection Decay")
                    .withID(140)
                    .asPercent()
                    .withDefault(0.5f)),
          angle(floatMd()
                    .withName("Reflection Angle")
                    .withID(150)
                    .withRange(0., 1.)
                    .withLinearScaleFormatting("")
                    .withDefault(0.1f)),
          root(floatMd()
                   .withName("Root Frequency")
                   .withID(160)
                   .withRange(0., 8.)
                   .withDefault(2.7279248)
                   .withATwoToTheBFormatting(4, 1, "hz")),
          spring(floatMd()
                   .withName("Spring Stiffness")
                   .withID(170)
                   .asPercent()
                   .withDefault(0.5)),
          mix(floatMd()
                   .withName("Mix")
                   .withID(180)
                   .withRange(0.0, 1.0)
                   .withLinearScaleFormatting("")
                   .withDefault(1.0))

    {
        this->pushSingleParam(&airflow);
        this->pushSingleParam(&vortex);
        this->pushSingleParam(&width);
        this->pushSingleParam(&center);
        this->pushSingleParam(&decay);
        this->pushSingleParam(&angle);
        this->pushSingleParam(&root);
        this->pushSingleParam(&spring);
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
} // namespace sapphire_plugins::tube_unit

#endif
