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

#ifndef SAPPHIRE_PLUGINS_ELASTIKA_ELASTIKA_H
#define SAPPHIRE_PLUGINS_ELASTIKA_ELASTIKA_H
#include <clap/clap.h>

namespace sapphire_plugins::elastika
{
const clap_plugin *makePlugin(const clap_host *);
const clap_plugin_descriptor *getDescriptor();

static constexpr const char *pluginId{"org.sapphire_plugin.elastika"};

} // namespace sapphire_plugins::elastika
#endif