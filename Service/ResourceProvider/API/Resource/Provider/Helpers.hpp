#pragma once

/// This file provides quality of life improvements that are not de factor part of ResourceProvider API.
/// They might need additional libraries to be included in order to satisfy all dependencies.
/// Also, all these functions are inline as they are build on top of the API and are not part of the implementation.

#include <Resource/Provider/ResourceProvider.hpp>

#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::Resource::Provider
{
/// \brief Adds directory, mounted using given configuration, to given resource provider as source.
inline SourceOperationResponse AddMountedDirectoryAsSource (ResourceProvider &_resourceProvider,
                                                            const VirtualFileSystem::Entry &_mountPoint,
                                                            const VirtualFileSystem::MountConfiguration &_configuration)
{
    const VirtualFileSystem::Entry mountedSource {_mountPoint, _configuration.targetPath};
    return _resourceProvider.AddSource (Memory::UniqueString {mountedSource.GetFullPath ()});
}

/// \brief Attempts to add all directories, mounted using given
///        configuration list, to given resource provider as sources.
inline SourceOperationResponse AddMountedDirectoriesAsSources (ResourceProvider &_resourceProvider,
                                                               const VirtualFileSystem::Entry &_mountPoint,
                                                               const VirtualFileSystem::MountConfigurationList &_list)
{
    for (const auto &configuration : _list.items)
    {
        if (SourceOperationResponse response =
                AddMountedDirectoryAsSource (_resourceProvider, _mountPoint, configuration);
            response != SourceOperationResponse::SUCCESSFUL)
        {
            return response;
        }
    }

    return SourceOperationResponse::SUCCESSFUL;
}
} // namespace Emergence::Resource::Provider
