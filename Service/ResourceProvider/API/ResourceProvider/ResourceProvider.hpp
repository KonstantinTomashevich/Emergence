#pragma once

#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/MappingRegistry.hpp>

#include <Memory/Heap.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::ResourceProvider
{
enum class SourceOperationResponse : uint8_t
{
    SUCCESSFUL = 0u,
    ALREADY_EXIST,
    NOT_FOUND,
    IO_ERROR,
    UNKNOWN_TYPE,
    UNKNOWN_FORMAT,
    DUPLICATE_ID,
};

enum class LoadingOperationResponse : uint8_t
{
    SUCCESSFUL = 0u,
    NOT_FOUND,
    IO_ERROR,
    WRONG_TYPE,
};

class ResourceProvider final
{
public:
    class ObjectRegistryCursor final
    {
    public:
        ObjectRegistryCursor (const ObjectRegistryCursor &_other) noexcept;

        ObjectRegistryCursor (ObjectRegistryCursor &&_other) noexcept;

        ~ObjectRegistryCursor () noexcept;

        [[nodiscard]] Memory::UniqueString operator* () const noexcept;

        ObjectRegistryCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (ObjectRegistryCursor);

    private:
        friend class ResourceProvider;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 4u);

        explicit ObjectRegistryCursor (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    ResourceProvider (Container::MappingRegistry _objectTypesRegistry,
                      Container::MappingRegistry _patchableTypesRegistry) noexcept;

    ResourceProvider (const ResourceProvider &_other) = delete;

    ResourceProvider (ResourceProvider &&_other) = delete;

    ~ResourceProvider () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceProvider);

    [[nodiscard]] const Container::MappingRegistry &GetObjectTypesRegistry () const noexcept;

    [[nodiscard]] const Container::MappingRegistry &GetPatchableTypesRegistry () const noexcept;

    [[nodiscard]] SourceOperationResponse AddSource (Memory::UniqueString _path) noexcept;

    [[nodiscard]] SourceOperationResponse SaveSourceIndex (Memory::UniqueString _sourcePath) const noexcept;

    [[nodiscard]] SourceOperationResponse RemoveSource (Memory::UniqueString _path) noexcept;

    [[nodiscard]] LoadingOperationResponse LoadObject (const StandardLayout::Mapping &_type,
                                                       Memory::UniqueString _id,
                                                       void *_output) const noexcept;

    [[nodiscard]] LoadingOperationResponse LoadThirdPartyResource (Memory::UniqueString _id,
                                                                   Memory::Heap &_allocator,
                                                                   uint64_t &_sizeOutput,
                                                                   uint8_t *&_dataPointerOutput) const noexcept;

    [[nodiscard]] ObjectRegistryCursor FindObjectsByType (const StandardLayout::Mapping &_type) const noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);
};
} // namespace Emergence::ResourceProvider
