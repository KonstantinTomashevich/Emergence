#include <Resource/Provider/Helpers.hpp>

#include <Testing/ResourceContextHolder.hpp>
#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Helpers.hpp>

#undef CreateDirectory

namespace Emergence::Testing
{
ResourceContextHolder &ResourceContextHolder::Get () noexcept
{
    static ResourceContextHolder contextHolder;
    return contextHolder;
}

ResourceContextHolder::ResourceContextHolder () noexcept
    : resourceProvider (&virtualFileSystem, GetSupportedResourceTypes (), {})
{
    {
        Emergence::VirtualFileSystem::MountConfigurationList configurationList;
        REQUIRE (Emergence::VirtualFileSystem::FetchMountConfigurationList (".", "CoreResources", configurationList));

        const VirtualFileSystem::Entry resourcesDirectory =
            virtualFileSystem.CreateDirectory (virtualFileSystem.GetRoot (), RESOURCES_DIRECTORY);

        REQUIRE (Emergence::VirtualFileSystem::MountConfigurationListAt (virtualFileSystem, resourcesDirectory,
                                                                         configurationList));

        REQUIRE (Emergence::Resource::Provider::AddMountedDirectoriesAsSources (resourceProvider, resourcesDirectory,
                                                                                configurationList) ==
                 Resource::Provider::SourceOperationResponse::SUCCESSFUL);
    }

    {
        Emergence::VirtualFileSystem::MountConfigurationList configurationList;
        REQUIRE (Emergence::VirtualFileSystem::FetchMountConfigurationList (".", "TestResources", configurationList));

        const VirtualFileSystem::Entry testDirectory =
            virtualFileSystem.CreateDirectory (virtualFileSystem.GetRoot (), TEST_DIRECTORY);

        REQUIRE (Emergence::VirtualFileSystem::MountConfigurationListAt (virtualFileSystem, testDirectory,
                                                                         configurationList));
    }

    REQUIRE (virtualFileSystem.Mount (virtualFileSystem.GetRoot (),
                                      {VirtualFileSystem::MountSource::FILE_SYSTEM, ".", TEST_OUTPUT_DIRECTORY}));
}
} // namespace Emergence::Testing
