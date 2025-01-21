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

#include "configuration.h"
#include "sst/plugininfra/version_information.h"
#include "clap/sapphire-clap-entry-impl.h"
#include "clap/plugin.h"
#include "clapwrapper/vst3.h"
#include "clapwrapper/auv2.h"

#include <iostream>
#include <cstring>
#include <string.h>
#include <clap/clap.h>

#include "elastika/elastika.h"
#include "tube_unit/tube_unit.h"
#include "gravy/gravy.h"

namespace sapphire_plugins
{

uint32_t clap_get_plugin_count(const clap_plugin_factory *) { return 3; };
const clap_plugin_descriptor *clap_get_plugin_descriptor(const clap_plugin_factory *f, uint32_t w)
{
    if (w == 0)
    {
        return elastika::getDescriptor();
    }

    if (w == 1)
    {
        return tube_unit::getDescriptor();
    }

    if (w == 2)
    {
        return gravy::getDescriptor();
    }

    return nullptr;
}

const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                      const char *plugin_id)
{
    if (strcmp(plugin_id, elastika::getDescriptor()->id) == 0)
    {
        return elastika::makePlugin(host);
    }
    if (strcmp(plugin_id, tube_unit::getDescriptor()->id) == 0)
    {
        return tube_unit::makePlugin(host);
    }

    if (strcmp(plugin_id, gravy::getDescriptor()->id) == 0)
    {
        return gravy::makePlugin(host);
    }
    return nullptr;
}

static bool clap_get_auv2_info(const clap_plugin_factory_as_auv2 *factory, uint32_t index,
                               clap_plugin_info_as_auv2_t *info)
{
    if (index == 0)
    {
        strncpy(info->au_type, "aufx", 5); // use the features to determine the type
        strncpy(info->au_subt, "elas", 5);

        return true;
    }

    if (index == 1)
    {
        strncpy(info->au_type, "aufx", 5); // use the features to determine the type
        strncpy(info->au_subt, "tbun", 5);

        return true;
    }

    if (index == 2)
    {
        strncpy(info->au_type, "aufx", 5); // use the features to determine the type
        strncpy(info->au_subt, "grvy", 5);

        return true;
    }

    return false;
}

static const clap_plugin_info_as_vst3 *clap_get_vst3_info(const clap_plugin_factory_as_vst3 *f,
                                                          uint32_t index)
{
    return nullptr;
}

const void *get_factory(const char *factory_id)
{
    SPLLOG("Asking for factory [" << factory_id << "]");
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0)
    {
        static const struct clap_plugin_factory six_sines_clap_factory = {
            clap_get_plugin_count,
            clap_get_plugin_descriptor,
            clap_create_plugin,
        };
        return &six_sines_clap_factory;
    }
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_INFO_AUV2) == 0)
    {
        static const struct clap_plugin_factory_as_auv2 six_sines_auv2_factory = {
            "SPhR",    // manu
            "Saphire", // manu name
            clap_get_auv2_info};
        return &six_sines_auv2_factory;
    }
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_INFO_VST3) == 0)
    {
        static const struct clap_plugin_factory_as_vst3 six_sines_vst3_factory = {
            "Sapphire", "", "", clap_get_vst3_info};

        return &six_sines_vst3_factory;
    }
    return nullptr;
}
bool clap_init(const char *p)
{
    // sst::plugininfra::misc_platform::allocateConsole();
    SPLLOG("Initializing Sapphire");
    return true;
}
void clap_deinit() {}
} // namespace sapphire_plugins