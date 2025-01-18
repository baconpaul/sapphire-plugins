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

namespace sapphire_plugins
{
const void *get_factory(const char *factory_id);
bool clap_init(const char *p);
void clap_deinit();
} // namespace sapphire_plugins
