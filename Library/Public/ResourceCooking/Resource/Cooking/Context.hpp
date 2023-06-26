#pragma once

#include <Resource/Cooking/ResourceList.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::Resource::Cooking
{
// TODO: Later we might need advanced is-update-needed check for intermediate cache.
//       Right now passes are doing that by themselves (not always even) and do not take cooker version into account.
//       This needs careful consideration, because excessive is-update-needed checks might severely slow down
//       cooking and cause more harm that good.

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

    const ResourceList &GetResourceList () const noexcept;

    ResourceList &GetResourceList () noexcept;

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
    ResourceList resourceList;
};
} // namespace Emergence::Resource::Cooking
