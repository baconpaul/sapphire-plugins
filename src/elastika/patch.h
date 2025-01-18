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

#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <clap/clap.h>
#include "configuration.h"
#include "elastika.h"
#include "sst/cpputils/constructors.h"
#include "sst/basic-blocks/params/ParamMetadata.h"
#include "sst/basic-blocks/dsp/Lag.h"
#include "sst/plugininfra/patch-support/patch_base.h"

namespace sapphire_plugins::elastika
{
namespace scpu = sst::cpputils;
namespace pats = sst::plugininfra::patch_support;
using md_t = sst::basic_blocks::params::ParamMetaData;
struct Param : pats::ParamBase
{
    Param(const md_t &m) : pats::ParamBase(m) {}

    Param &operator=(const float &val)
    {
        value = val;
        return *this;
    }

    sst::basic_blocks::dsp::OnePoleLag<float, true> lag;
    void snap() { lag.snapTo(value); }
};

struct Patch : pats::PatchBase<Patch, Param>
{
    static constexpr uint32_t patchVersion{1};
    static constexpr const char *id{pluginId};
    char name[256]{""};

    static constexpr uint32_t floatFlags{CLAP_PARAM_IS_AUTOMATABLE};

    static md_t floatMd() { return md_t().asFloat().withFlags(floatFlags); }

    Param friction, stiffness, span, curl, mass, drive, level, inputTilt, outputTilt;

    Patch()
        : pats::PatchBase<Patch, Param>(),
          friction(floatMd().withName("Friction").withID(100).asPercent().withDefault(0.5)),
          span(floatMd().withName("Span").withID(110).asPercent().withDefault(0.5)),
          stiffness(floatMd().withName("Stiffness").withID(120).asPercent().withDefault(0.5)),
          curl(floatMd().withName("Curl").withID(130).asPercentBipolar().withDefault(0.f)),
          mass(floatMd().withName("Mass").withID(140).asPercentBipolar().withDefault(0.f)),
          drive(floatMd()
                    .withName("Drive")
                    .withID(150)
                    .withRange(0., 2.)
                    .withLinearScaleFormatting("")
                    .withDefault(1.f)),
          level(floatMd()
                    .withName("Level")
                    .withID(160)
                    .withRange(0., 2.)
                    .withLinearScaleFormatting("")
                    .withDefault(1.f)),
          inputTilt(floatMd().withName("Input Tilt").withID(170).asPercent().withDefault(0.5)),
          outputTilt(floatMd().withName("Output Tilt").withID(180).asPercent().withDefault(0.5))
    {
        this->pushSingleParam(&friction);
        this->pushSingleParam(&stiffness);
        this->pushSingleParam(&span);
        this->pushSingleParam(&curl);
        this->pushSingleParam(&mass);
        this->pushSingleParam(&drive);
        this->pushSingleParam(&level);
        this->pushSingleParam(&inputTilt);
        this->pushSingleParam(&outputTilt);

        resetToInit();
        for (auto &[i, p] : paramMap)
        {
            p->snap();
        }
    }

    void migratePatchFromVersion(uint32_t) {}
    float migrateParamValueFromVersion(Param *, float v, uint32_t) { return v; }
};
} // namespace sapphire_plugins::elastika
