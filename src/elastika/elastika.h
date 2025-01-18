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