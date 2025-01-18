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

#ifndef SAPPHIRE_PLUGINS_SHARED_PARAM_WITH_LAG_H
#define SAPPHIRE_PLUGINS_SHARED_PARAM_WITH_LAG_H

#include "sst/plugininfra/patch-support/patch_base.h"
#include "sst/basic-blocks/params/ParamMetadata.h"
#include "sst/basic-blocks/dsp/Lag.h"

namespace sapphire_plugins::shared
{
struct ParamWithLag : sst::plugininfra::patch_support::ParamBase
{
    ParamWithLag(const sst::basic_blocks::params::ParamMetaData &m)
        : sst::plugininfra::patch_support::ParamBase(m)
    {
    }

    ParamWithLag &operator=(const float &val)
    {
        value = val;
        return *this;
    }

    sst::basic_blocks::dsp::OnePoleLag<float, true> lag;
    void snap() { lag.snapTo(value); }
};
} // namespace sapphire_plugins::shared
#endif // PARAM_WITH_LAG_H
