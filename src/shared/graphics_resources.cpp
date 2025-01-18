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

} // namespace sapphire_plugins::shared