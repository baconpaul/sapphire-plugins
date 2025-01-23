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

#include <cmath>
#include <optional>
#include <string>
#include "sapphire_panel.hpp"

namespace sapphire_plugins::shared
{
std::optional<std::string> getSvgForPath(const std::string &path);

struct PanelDimensions
{
    int width;
    int height;

    explicit PanelDimensions(int _width, int _height)
        : width(_width)
        , height(_height)
        {}
};

PanelDimensions getPanelDimensions(
    const std::string& modcode,
    int widthCorrection    // FIXFIXFIX: this is a temporary hack to prevent black gaps in the plugin window
);
}

#endif // GRAPHICS_RESOURCES_H
