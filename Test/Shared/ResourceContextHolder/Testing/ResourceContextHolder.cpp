#include <filesystem>
#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Serialization/Yaml.hpp>

#include <Testing/ResourceContextHolder.hpp>
#include <Testing/Testing.hpp>

#undef CreateDirectory

namespace Emergence::Testing
{
ResourceContextHolder &ResourceContextHolder::Get () noexcept
{
    static ResourceContextHolder contextHolder;
    return contextHolder;
}

ResourceContextHolder::ResourceContextHolder () noexcept
    : resourceProvider (&virtualFileSystem, GetSupportedResourceTypes(), {})
{
    constexpr const char *CORE_RESOURCES_FILE = "MountCoreResources.yaml";
    constexpr const char *TEST_RESOURCES_FILE = "MountTestResources.yaml";

    if (std::filesystem::is_regular_file (CORE_RESOURCES_FILE))
    {
        VirtualFileSystem::MountConfigurationList configurationList;
        {
            std::ifstream input {CORE_RESOURCES_FILE};
            Serialization::Yaml::DeserializeObject (input, &configurationList,
                                                    VirtualFileSystem::MountConfigurationList::Reflect ().mapping, {});
        }

        const VirtualFileSystem::Entry resourcesDirectory =
            virtualFileSystem.CreateDirectory (virtualFileSystem.GetRoot (), RESOURCES_DIRECTORY);

        for (const auto &configuration : configurationList.items)
        {
            REQUIRE (virtualFileSystem.Mount (resourcesDirectory, configuration));
            REQUIRE (resourceProvider.AddSource (Emergence::Memory::UniqueString {EMERGENCE_BUILD_STRING (
                         RESOURCES_DIRECTORY, VirtualFileSystem::PATH_SEPARATOR, configuration.targetPath)}) ==
                     Resource::Provider::SourceOperationResponse::SUCCESSFUL);
        }
    }

    if (std::filesystem::is_regular_file (TEST_RESOURCES_FILE))
    {
        VirtualFileSystem::MountConfigurationList configurationList;
        {
            std::ifstream input {TEST_RESOURCES_FILE};
            Serialization::Yaml::DeserializeObject (input, &configurationList,
                                                    VirtualFileSystem::MountConfigurationList::Reflect ().mapping, {});
        }

        const VirtualFileSystem::Entry testDirectory =
            virtualFileSystem.CreateDirectory (virtualFileSystem.GetRoot (), TEST_DIRECTORY);

        for (const auto &configuration : configurationList.items)
        {
            REQUIRE (virtualFileSystem.Mount (testDirectory, configuration));
        }
    }

    REQUIRE (virtualFileSystem.Mount (virtualFileSystem.GetRoot (),
                                      {VirtualFileSystem::MountSource::FILE_SYSTEM, ".", TEST_OUTPUT_DIRECTORY}));
}
} // namespace Emergence::Testing
