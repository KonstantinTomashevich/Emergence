#pragma once

#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Searches for mount list of given group in given path and attempts to deserialize it.
inline bool FetchMountConfigurationList (const std::string_view &_pathToLook,
                                         const std::string_view &_group,
                                         MountConfigurationList &_output)
{
    // We start by looking for YAML as priority in order to make game modification support easier.
    const Container::Utf8String yamlPath =
        EMERGENCE_BUILD_STRING (_pathToLook, PATH_SEPARATOR, "Mount", _group, ".yaml");

    if (std::ifstream input {yamlPath.c_str (), std::ios::binary})
    {
        return Serialization::Yaml::DeserializeObject (input, &_output, MountConfigurationList::Reflect ().mapping, {});
    }

    const Container::Utf8String binaryPath =
        EMERGENCE_BUILD_STRING (_pathToLook, PATH_SEPARATOR, "Mount", _group, ".bin");

    if (std::ifstream input {binaryPath.c_str (), std::ios::binary})
    {
        return Serialization::Binary::DeserializeObject (input, &_output, MountConfigurationList::Reflect ().mapping,
                                                         {});
    }

    return false;
}

/// \brief Attempts to mount all entries from given mount list.
inline bool MountConfigurationListAt (Context &_context, const Entry &_at, const MountConfigurationList &_list)
{
    for (const MountConfiguration &configuration : _list.items)
    {
        if (!_context.Mount (_at, configuration))
        {
            return false;
        }
    }

    return true;
}
} // namespace Emergence::VirtualFileSystem
