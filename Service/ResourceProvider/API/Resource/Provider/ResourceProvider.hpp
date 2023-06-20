#pragma once

#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/MappingRegistry.hpp>

#include <Memory/Heap.hpp>

#include <StandardLayout/Mapping.hpp>

#include <VirtualFileSystem/Context.hpp>

namespace Emergence::Resource::Provider
{
/// \brief Exit codes for ResourceProvider operations that work with resource sources.
enum class SourceOperationResponse : std::uint8_t
{
    /// \brief Executed successfully.
    SUCCESSFUL = 0u,

    /// \brief Source already exists and cannot be added again.
    ALREADY_EXIST,

    /// \brief Required source is not found.
    NOT_FOUND,

    /// \brief Operation failed due to IO error.
    IO_ERROR,

    /// \brief Found resource object of unknown type.
    UNKNOWN_TYPE,

    /// \brief Found resource object of unknown format (not binary or yaml).
    UNKNOWN_FORMAT,

    /// \brief Resource object or third party resource id duplication detected.
    DUPLICATE_ID,
};

/// \brief Exit codes for ResourceProvider operations that load resources.
enum class LoadingOperationResponse : std::uint8_t
{
    /// \brief Executed successfully.
    SUCCESSFUL = 0u,

    /// \brief Resource object or third party resource with given id was not found.
    NOT_FOUND,

    /// \brief Operation failed due to IO error.
    IO_ERROR,

    /// \brief Requested resource object has different type than requested.
    WRONG_TYPE,
};

/// \brief Encapsulates resource discovery logic and provides unified access
///        to reflection-driven and third party resources.
///
/// \par Resource types
/// \parblock
/// Resource is called reflection driven if it can be serialized and deserialized through Serialization library API
/// using StandardLayout::Mapping's. ResourceProvider expects these files to have "*.bin" or ".yaml" extensions to
/// identify their format. Type of the resource is discovered by reading the first null-terminated string (for binary
/// format) or the first comment (for yaml format). Id of such resource is file name without last extension.
///
/// All other resources are called third party. Their id is just their filename. No further detection logic is executed
/// for this type of resources.
/// \endparblock
///
/// \par Thread safety
/// \parblock
/// You can safely work with resource provider from multiple threads if you follow readers-writers principle: you can
/// safely execute several const methods from different threads, but no other methods can be executed while non-const
/// method is being executed.
/// \endparblock
class ResourceProvider final
{
public:
    /// \brief Cursor that provides access to ids of all resource objects of given type.
    class ObjectRegistryCursor final
    {
    public:
        ObjectRegistryCursor (const ObjectRegistryCursor &_other) noexcept;

        ObjectRegistryCursor (ObjectRegistryCursor &&_other) noexcept;

        ~ObjectRegistryCursor () noexcept;

        /// \brief Id of object to which cursor points right now.
        [[nodiscard]] Memory::UniqueString operator* () const noexcept;

        /// \brief Moves cursor to next object.
        ObjectRegistryCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (ObjectRegistryCursor);

    private:
        friend class ResourceProvider;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 8u);

        explicit ObjectRegistryCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Constructs resource provider that supports given resource types and is aware of given patchable types.
    ResourceProvider (VirtualFileSystem::Context *_virtualFileSystemContext,
                      Container::MappingRegistry _objectTypesRegistry,
                      Container::MappingRegistry _patchableTypesRegistry) noexcept;

    ResourceProvider (const ResourceProvider &_other) = delete;

    ResourceProvider (ResourceProvider &&_other) = delete;

    ~ResourceProvider () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceProvider);

    /// \return Supported resource object types.
    [[nodiscard]] const Container::MappingRegistry &GetObjectTypesRegistry () const noexcept;

    /// \brief Supported patchable types.
    [[nodiscard]] const Container::MappingRegistry &GetPatchableTypesRegistry () const noexcept;

    /// \brief Registers given source and adds all resources from it to resource provider.
    [[nodiscard]] SourceOperationResponse AddSource (Memory::UniqueString _path) noexcept;

    /// \brief Saves index file for given source to its location.
    [[nodiscard]] SourceOperationResponse SaveSourceIndex (Memory::UniqueString _sourcePath) const noexcept;

    /// \brief Removes given source and all resources associated with it.
    [[nodiscard]] SourceOperationResponse RemoveSource (Memory::UniqueString _path) noexcept;

    /// \brief Attempts to load reflection-based resource object by given id.
    /// \invariant Output must point to initialized object of requested type.
    [[nodiscard]] LoadingOperationResponse LoadObject (const StandardLayout::Mapping &_type,
                                                       Memory::UniqueString _id,
                                                       void *_output) const noexcept;

    /// \brief Attempts to fully load third party resource by its id, using given heap to allocate memory for it.
    [[nodiscard]] LoadingOperationResponse LoadThirdPartyResource (Memory::UniqueString _id,
                                                                   Memory::Heap &_allocator,
                                                                   std::uint64_t &_sizeOutput,
                                                                   std::uint8_t *&_dataOutput) const noexcept;

    /// \brief Returns cursor that provides access to ids of all resources of given type.
    /// \warning Cursor holds read access to resource registry while it is alive.
    [[nodiscard]] ObjectRegistryCursor FindObjectsByType (const StandardLayout::Mapping &_type) const noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);
};
} // namespace Emergence::Resource::Provider
