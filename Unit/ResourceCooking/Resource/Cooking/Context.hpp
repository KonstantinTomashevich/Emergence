#pragma once

#include <ResourceCookingApi.hpp>

#include <Resource/Cooking/ResourceList.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::Resource::Cooking
{
// TODO: Later we might need advanced is-update-needed check for intermediate cache.
//       Right now passes are doing that by themselves (not always even) and do not take cooker version into account.
//       This needs careful consideration, because excessive is-update-needed checks might severely slow down
//       cooking and cause more harm that good.

/// \brief Initializes resource cooking pipeline and stores current state of it.
///
/// \par File system
/// \parblock
/// There are 3 top level directories in virtual file system, managed by this class instance:
///
/// - Input: under this directory everything from input mount list is mounted. Also, every mounted entry is
///   automatically added to managed resource provider as source.
///
/// - Intermediate: this directory functions as storage for processing intermediate files, for example some caches
///   or transitory conversion results. It has 2 subdirectories:
///     - Real: points to directory in real file system that can be used for storing intermediate files.
///     - Virtual: points to directory in virtual file system that is suitable for tasks like modeling final or
///       transitory resource tree using weak file links and virtual directories.
///
/// - FinalResult: points to location in real file system under which resource cooking final results should be stored.
/// \endparblock
///
/// \par Passes and results
/// \parblock
/// Although context does not enforce any particular architecture, it is advised to use passes and results approach.
/// Pass is a single meaningful transformation, that is applied to resource list. It might be conversion from one
/// format to another, index backing or even just resource list population from resource provider. Passes are executed
/// sequentially and execution stops after first failure. Results are functions that take current resource list state
/// and use it to produce final cooking result, for example to build read-only package. They are usually executed
/// after all passes have been finished. This is a simple and straightforward solution, but it is easy to understand,
/// easy to extend and easy to modify.
/// \endparblock
class ResourceCookingApi Context final
{
public:
    /// \brief Constructs context with given object types registry and patchable types registry for ResourceProvider.
    Context (Container::MappingRegistry _objectTypesRegistry,
             Container::MappingRegistry _patchableTypesRegistry) noexcept;

    Context (const Context &_other) = delete;

    Context (Context &&_other) = delete;

    ~Context () noexcept = default;

    /// \brief Attempts to setup resource cooking pipeline by mounting given input and setting up for given workspace.
    bool Setup (const VirtualFileSystem::MountConfigurationList &_inputMount,
                const std::string_view &_workspaceRealPath) noexcept;

    /// \return Reference to ongoing resource list.
    const ResourceList &GetResourceList () const noexcept;

    /// \return Reference to ongoing resource list.
    ResourceList &GetResourceList () noexcept;

    /// \return Reference to resource provider that has scanned all the mounted inputs.
    const Provider::ResourceProvider &GetInitialResourceProvider () const noexcept;

    /// \return Reference to resource provider that has scanned all the mounted inputs.
    Provider::ResourceProvider &GetInitialResourceProvider () noexcept;

    /// \return Reference to virtual file system instance that is used by this context.
    const VirtualFileSystem::Context &GetVirtualFileSystem () const noexcept;

    /// \return Reference to virtual file system instance that is used by this context.
    VirtualFileSystem::Context &GetVirtualFileSystem () noexcept;

    /// \return Entry of Input root directory, see "File system" paragraph.
    VirtualFileSystem::Entry GetInputDirectory () const noexcept;

    /// \return Entry of Intermediate/Real directory, see "File system" paragraph.
    VirtualFileSystem::Entry GetIntermediateRealDirectory () const noexcept;

    /// \return Entry of Intermediate/Virtual directory, see "File system" paragraph.
    VirtualFileSystem::Entry GetIntermediateVirtualDirectory () const noexcept;

    /// \brief Entry of directory under Intermediate/Real, specially created for pass with given name.
    VirtualFileSystem::Entry GetPassIntermediateRealDirectory (const std::string_view &_passName) noexcept;

    /// \brief Entry of directory under Intermediate/Virtual, specially created for pass with given name.
    VirtualFileSystem::Entry GetPassIntermediateVirtualDirectory (const std::string_view &_passName) noexcept;

    /// \return Entry of FinalResult root directory, see "File system" paragraph.
    VirtualFileSystem::Entry GetFinalResultDirectory () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Context);

private:
    VirtualFileSystem::Context virtualFileSystem;
    Provider::ResourceProvider resourceProvider;
    ResourceList resourceList;
};
} // namespace Emergence::Resource::Cooking
