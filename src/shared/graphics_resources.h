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

#ifndef SAPPHIRE_PLUGINS_SHARED_GRAPHICS_RESOURCES_H
#define SAPPHIRE_PLUGINS_SHARED_GRAPHICS_RESOURCES_H

#include <optional>
#include <string>

namespace sapphire_plugins::shared
{
std::optional<std::string> getSvgForPath(const std::string &path);
}

#endif // GRAPHICS_RESOURCES_H
