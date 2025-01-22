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

#include "graphics_resources.h"
#include "configuration.h"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(sapphire_graphics);

namespace sapphire_plugins::shared
{
std::optional<std::string> getSvgForPath(const std::string &path)
{
    SPLLOG("Loading graphic from '" << path << "'");

    try
    {
        auto fs = cmrc::sapphire_graphics::get_filesystem();

        if (fs.exists(path))
        {
            auto f = fs.open(path);
            return std::string(f.begin(), f.end());
        }
    }
    catch (const std::exception &e)
    {
        SPLLOG(e.what());
    }
    return std::nullopt;
    ;
}

PanelDimensions getPanelDimensions(const std::string& modcode)
{
    // Get panel dimensions in VCV Rack millimeter units.
    auto mmPanel = Sapphire::GetPanelDimensions(modcode);

    // Convert millimeters to pixels.
    const float pixelsPerMillimeter = 600 / 128.5;
    int dx = static_cast<int>(std::round(pixelsPerMillimeter * mmPanel.cx));
    int dy = static_cast<int>(std::round(pixelsPerMillimeter * mmPanel.cy));
    return PanelDimensions(dx, dy);
}

} // namespace sapphire_plugins::shared