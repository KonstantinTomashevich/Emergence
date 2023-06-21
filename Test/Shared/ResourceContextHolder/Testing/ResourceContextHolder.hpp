#pragma once

#include <cstdint>

#include <Resource/Provider/ResourceProvider.hpp>

struct SDL_Window;

namespace Emergence::Testing
{
/// \brief Returns registry with all the resource types, supported by backend that uses ResourceContextHolder.
/// \warning Test-specific, therefore should be implemented on user side.
Container::MappingRegistry GetSupportedResourceTypes () noexcept;

/// \brief Holds and initializes virtual file system and resource provider using mount lists in working directory.
class ResourceContextHolder final
{
public:
    /// \brief Virtual directory, child of root, under which all `CoreResources` will be stored.
    static constexpr const char *RESOURCES_DIRECTORY = "Resources";

    /// \brief Virtual directory, child of root, under which all `TestResources` will be stored.
    static constexpr const char *TEST_DIRECTORY = "Test";

    /// \brief Virtual directory, child of root, that points to test working directory.
    static constexpr const char *TEST_OUTPUT_DIRECTORY = "TestOutput";

    /// \return Singleton instance of this class.
    static ResourceContextHolder &Get () noexcept;

    ResourceContextHolder (const ResourceContextHolder &_other) = delete;

    ResourceContextHolder (ResourceContextHolder &&_other) = delete;

    ResourceContextHolder &operator= (const ResourceContextHolder &_other) = delete;

    ResourceContextHolder &operator= (ResourceContextHolder &&_other) = delete;

    /// \brief Managed virtual file system context instance.
    VirtualFileSystem::Context virtualFileSystem;

    /// \brief Managed resource provider instance.
    Resource::Provider::ResourceProvider resourceProvider;

private:
    ResourceContextHolder () noexcept;

    ~ResourceContextHolder () noexcept = default;
};
} // namespace Emergence::Testing
