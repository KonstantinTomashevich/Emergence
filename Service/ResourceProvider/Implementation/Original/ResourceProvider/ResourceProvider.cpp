#include <Assert/Assert.hpp>

#include <ResourceProvider/Original/ResourceProvider.hpp>
#include <ResourceProvider/ResourceProvider.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::ResourceProvider
{
ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (
    const ResourceProvider::ObjectRegistryCursor &_other) noexcept
{
    new (&data) Original::ResourceProvider::ObjectRegistryCursor (
        block_cast<Original::ResourceProvider::ObjectRegistryCursor> (_other.data));
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (ResourceProvider::ObjectRegistryCursor &&_other) noexcept
{
    new (&data) Original::ResourceProvider::ObjectRegistryCursor (
        std::move (block_cast<Original::ResourceProvider::ObjectRegistryCursor> (_other.data)));
}

ResourceProvider::ObjectRegistryCursor::~ObjectRegistryCursor () noexcept
{
    block_cast<Original::ResourceProvider::ObjectRegistryCursor> (data).~ObjectRegistryCursor ();
}

Memory::UniqueString ResourceProvider::ObjectRegistryCursor::operator* () const noexcept
{
    return *block_cast<Original::ResourceProvider::ObjectRegistryCursor> (data);
}

ResourceProvider::ObjectRegistryCursor &ResourceProvider::ObjectRegistryCursor::operator++ () noexcept
{
    ++block_cast<Original::ResourceProvider::ObjectRegistryCursor> (data);
    return *this;
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::ResourceProvider::ObjectRegistryCursor (
        std::move (block_cast<Original::ResourceProvider::ObjectRegistryCursor> (_data)));
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"ResourceProvider"}}};
    Original::ResourceProvider *resourceProvider = nullptr;
};

ResourceProvider::ResourceProvider (Container::MappingRegistry _objectTypesRegistry,
                                    Container::MappingRegistry _patchableTypesRegistry) noexcept
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();

    internal.resourceProvider =
        new (internal.heap.Acquire (sizeof (Original::ResourceProvider), alignof (Original::ResourceProvider)))
            Original::ResourceProvider (std::move (_objectTypesRegistry), std::move (_patchableTypesRegistry));
}

ResourceProvider::~ResourceProvider () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    if (internal.resourceProvider)
    {
        internal.resourceProvider->~ResourceProvider ();
        internal.heap.Release (internal.resourceProvider, sizeof (Original::ResourceProvider));
    }

    internal.~InternalData ();
}

const Container::MappingRegistry &ResourceProvider::GetObjectTypesRegistry () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->GetObjectTypesRegistry ();
}

const Container::MappingRegistry &ResourceProvider::GetPatchableTypesRegistry () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->GetPatchableTypesRegistry ();
}

SourceOperationResponse ResourceProvider::AddSource (Memory::UniqueString _path) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->AddSource (_path);
}

SourceOperationResponse ResourceProvider::SaveSourceIndex (Memory::UniqueString _sourcePath) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->SaveSourceIndex (_sourcePath);
}

SourceOperationResponse ResourceProvider::RemoveSource (Memory::UniqueString _path) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->RemoveSource (_path);
}

LoadingOperationResponse ResourceProvider::LoadObject (const StandardLayout::Mapping &_type,
                                                       Memory::UniqueString _id,
                                                       void *_output) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->LoadObject (_type, _id, _output);
}

LoadingOperationResponse ResourceProvider::LoadThirdPartyResource (Memory::UniqueString _id,
                                                                   Memory::Heap &_allocator,
                                                                   uint64_t &_sizeOutput,
                                                                   uint8_t *&_dataPointerOutput) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->LoadThirdPartyResource (_id, _allocator, _sizeOutput, _dataPointerOutput);
}

ResourceProvider::ObjectRegistryCursor ResourceProvider::FindObjectsByType (
    const StandardLayout::Mapping &_type) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    Original::ResourceProvider::ObjectRegistryCursor cursor = internal.resourceProvider->FindObjectsByType (_type);
    return ObjectRegistryCursor (array_cast (cursor));
}
} // namespace Emergence::ResourceProvider
