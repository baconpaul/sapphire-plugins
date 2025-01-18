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

#ifndef SAPPHIRE_PLUGINS_SHARED_GRAPHICS_RESOURCES_H
#define SAPPHIRE_PLUGINS_SHARED_GRAPHICS_RESOURCES_H

#include <optional>
#include <string>

namespace sapphire_plugins::shared
{
std::optional<std::string> getSvgForPath(const std::string &path);
}

#endif // GRAPHICS_RESOURCES_H
