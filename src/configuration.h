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

#ifndef SAPPHIRE_PLUGINS_CONFIGURATION_H
#define SAPPHIRE_PLUGINS_CONFIGURATION_H

#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>

#include "sst/plugininfra/version_information.h"

inline std::string fileTrunc(const std::string &f)
{
    auto p = f.find(sst::plugininfra::VersionInformation::cmake_source_dir);
    if (p != std::string::npos)
    {
        return f.substr(p + strlen(sst::plugininfra::VersionInformation::cmake_source_dir) + 1);
    }
    return f;
}

#define SPLLOG(...)                                                                                \
    std::cout << fileTrunc(__FILE__) << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl;
#define SPLV(x) " " << #x << "=" << x

#endif // CONFIGURATION_H
