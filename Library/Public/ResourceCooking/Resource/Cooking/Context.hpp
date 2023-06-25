#pragma once

#include <Resource/Cooking/ResourceList.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::Resource::Cooking
{
class Context final
{
public:
    Context (Container::MappingRegistry _objectTypesRegistry,
             Container::MappingRegistry _patchableTypesRegistry) noexcept;

    Context (const Context &_other) = delete;

    Context (Context &&_other) = delete;

    ~Context () noexcept = default;

    bool Setup (const VirtualFileSystem::MountConfigurationList &_inputMount,
                const std::string_view &_workspaceRealPath) noexcept;

    void OnPassFinished () noexcept;

    // TODO: It seems that architecture that uses one list, but allows to delete items from it, is much more efficient.
    //       Investigate this idea later.

    const ResourceList &GetSourceList () const noexcept;

    ResourceList &GetTargetList () noexcept;

    Provider::ResourceProvider &GetInitialResourceProvider () noexcept;

    VirtualFileSystem::Context &GetVirtualFileSystem () noexcept;

    VirtualFileSystem::Entry GetInputDirectory () const noexcept;

    VirtualFileSystem::Entry GetIntermediateRealDirectory () const noexcept;

    VirtualFileSystem::Entry GetIntermediateVirtualDirectory () const noexcept;

    VirtualFileSystem::Entry GetPassIntermediateRealDirectory (const std::string_view &_passName) noexcept;

    VirtualFileSystem::Entry GetPassIntermediateVirtualDirectory (const std::string_view &_passName) noexcept;

    VirtualFileSystem::Entry GetFinalResultDirectory () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Context);

private:
    VirtualFileSystem::Context virtualFileSystem;
    Provider::ResourceProvider resourceProvider;

    ResourceList firstList;
    ResourceList secondList;

    ResourceList *sourceList = &firstList;
    ResourceList *targetList = &secondList;
};
} // namespace Emergence::Resource::Cooking
