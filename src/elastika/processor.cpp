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

#include "sst/plugininfra/version_information.h"
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
        "org.sapphire_plugin.elastika",
        "Elastika",
        "Sapphirel",
        "",
        "",
        "",
        sst::plugininfra::VersionInformation::project_version_and_hash,
        "Fun with Physical Modelling",
        &features[0]};
    return &desc;
}

const clap_plugin *makePlugin(const clap_host *) { return nullptr; }

} // namespace sapphire_plugins::elastika