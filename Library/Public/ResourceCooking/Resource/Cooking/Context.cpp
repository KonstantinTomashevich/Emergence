#include <filesystem>

#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Resource/Cooking/Context.hpp>

#include <Resource/Provider/Helpers.hpp>

#include <VirtualFileSystem/Helpers.hpp>

namespace Emergence::Resource::Cooking
{
static const char *const INPUT = "Input";
static const char *const INTERMEDIATE = "Intermediate";
static const char *const REAL = "Real";
static const char *const VIRTUAL = "Virtual";
static const char *const FINAL_RESULT = "FinalResult";

Context::Context (Container::MappingRegistry _objectTypesRegistry,
                  Container::MappingRegistry _patchableTypesRegistry) noexcept
    : resourceProvider (&virtualFileSystem, std::move (_objectTypesRegistry), std::move (_patchableTypesRegistry))
{
    [[maybe_unused]] const VirtualFileSystem::Entry inputEntry =
        virtualFileSystem.MakeDirectories (virtualFileSystem.GetRoot (), INPUT);
    EMERGENCE_ASSERT (inputEntry.GetType () == VirtualFileSystem::EntryType::DIRECTORY);

    [[maybe_unused]] const VirtualFileSystem::Entry intermediateVirtual = virtualFileSystem.MakeDirectories (
        virtualFileSystem.GetRoot (),
        EMERGENCE_BUILD_STRING (INTERMEDIATE, VirtualFileSystem::PATH_SEPARATOR, VIRTUAL));
    EMERGENCE_ASSERT (intermediateVirtual.GetType () == VirtualFileSystem::EntryType::DIRECTORY);
}

bool Context::Setup (const VirtualFileSystem::MountConfigurationList &_inputMount,
                     const std::string_view &_workspaceRealPath) noexcept
{
    const Container::Utf8String intermediateRealDirectory {
        EMERGENCE_BUILD_STRING (_workspaceRealPath, VirtualFileSystem::PATH_SEPARATOR, INTERMEDIATE)};

    const Container::Utf8String finalResultRealDirectory {
        EMERGENCE_BUILD_STRING (_workspaceRealPath, VirtualFileSystem::PATH_SEPARATOR, FINAL_RESULT)};

    if (!std::filesystem::is_directory (intermediateRealDirectory) &&
        !std::filesystem::create_directories (intermediateRealDirectory))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to ensure that \"", _workspaceRealPath, "\" contains \"",
                       INTERMEDIATE, "\" directory.");
        return false;
    }

    if (!std::filesystem::is_directory (finalResultRealDirectory) &&
        !std::filesystem::create_directories (finalResultRealDirectory))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to ensure that \"", _workspaceRealPath, "\" contains \"",
                       FINAL_RESULT, "\" directory.");
        return false;
    }

    if (!virtualFileSystem.Mount (VirtualFileSystem::Entry {virtualFileSystem.GetRoot (), INTERMEDIATE},
                                  {VirtualFileSystem::MountSource::FILE_SYSTEM, intermediateRealDirectory, REAL}))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to mount workspace intermediate directory \"",
                       intermediateRealDirectory, "\".");
        return false;
    }

    if (!virtualFileSystem.Mount (virtualFileSystem.GetRoot (), {VirtualFileSystem::MountSource::FILE_SYSTEM,
                                                                 finalResultRealDirectory, FINAL_RESULT}))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Unable to mount workspace final result directory \"",
                       finalResultRealDirectory, "\".");
        return false;
    }

    const VirtualFileSystem::Entry inputDirectory = GetInputDirectory ();
    if (!Emergence::VirtualFileSystem::MountConfigurationListAt (virtualFileSystem, inputDirectory, _inputMount))
    {
        EMERGENCE_LOG (ERROR, "Resource::Cooking: Failed to mount given input mount list!");
        return false;
    }

    if (Emergence::Resource::Provider::AddMountedDirectoriesAsSources (resourceProvider, inputDirectory, _inputMount) !=
        Emergence::Resource::Provider::SourceOperationResponse::SUCCESSFUL)
    {
        EMERGENCE_LOG (
            ERROR, "Resource::Cooking: Failed to add directories from given mount list as resource provider sources!");
        return false;
    }

    return true;
}

const ResourceList &Context::GetResourceList () const noexcept
{
    return resourceList;
}

ResourceList &Context::GetResourceList () noexcept
{
    return resourceList;
}

const Provider::ResourceProvider &Context::GetInitialResourceProvider () const noexcept
{
    return resourceProvider;
}

Provider::ResourceProvider &Context::GetInitialResourceProvider () noexcept
{
    return resourceProvider;
}

const VirtualFileSystem::Context &Context::GetVirtualFileSystem () const noexcept
{
    return virtualFileSystem;
}

VirtualFileSystem::Context &Context::GetVirtualFileSystem () noexcept
{
    return virtualFileSystem;
}

VirtualFileSystem::Entry Context::GetInputDirectory () const noexcept
{
    return {virtualFileSystem, INPUT};
}

VirtualFileSystem::Entry Context::GetIntermediateRealDirectory () const noexcept
{
    return {virtualFileSystem, EMERGENCE_BUILD_STRING (INTERMEDIATE, VirtualFileSystem::PATH_SEPARATOR, REAL)};
}

VirtualFileSystem::Entry Context::GetIntermediateVirtualDirectory () const noexcept
{
    return {virtualFileSystem, EMERGENCE_BUILD_STRING (INTERMEDIATE, VirtualFileSystem::PATH_SEPARATOR, VIRTUAL)};
}

VirtualFileSystem::Entry Context::GetPassIntermediateRealDirectory (const std::string_view &_passName) noexcept
{
    return virtualFileSystem.MakeDirectories (GetIntermediateRealDirectory (), _passName);
}

VirtualFileSystem::Entry Context::GetPassIntermediateVirtualDirectory (const std::string_view &_passName) noexcept
{
    return virtualFileSystem.MakeDirectories (GetIntermediateVirtualDirectory (), _passName);
}

VirtualFileSystem::Entry Context::GetFinalResultDirectory () const noexcept
{
    return {virtualFileSystem, FINAL_RESULT};
}
} // namespace Emergence::Resource::Cooking
