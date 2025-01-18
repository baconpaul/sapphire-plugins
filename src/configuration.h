//
// Created by Paul Walker on 1/17/25.
//

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

#include "sst/plugininfra/version.h"

inline std::string fileTrunc(const std::string &f)
{
    auto p = f.find(sst::plugininfra::VersionInformation::cmake_source_dir);
    if (p != std::string::npos)
    {
        return f.substr(p + strlen(sst::plugininfra::VersionInformation::cmake_source_dir) + 1);
    }
    return f;
}

#define SPLLOG(...)                                                                               \
std::cout << fileTrunc(__FILE__) << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl;
#define SPLV(x) " " << #x << "=" << x

#endif //CONFIGURATION_H
