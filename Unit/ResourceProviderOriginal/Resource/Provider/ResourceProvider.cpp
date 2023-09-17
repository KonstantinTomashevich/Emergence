#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

#include <Resource/Provider/Original/ResourceProvider.hpp>
#include <Resource/Provider/ResourceProvider.hpp>

namespace Emergence::Resource::Provider
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

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::ResourceProvider::ObjectRegistryCursor (
        std::move (block_cast<Original::ResourceProvider::ObjectRegistryCursor> (_data)));
}

ResourceProvider::ThirdPartyRegistryCursor::ThirdPartyRegistryCursor (
    const ResourceProvider::ThirdPartyRegistryCursor &_other) noexcept
{
    new (&data) Original::ResourceProvider::ThirdPartyRegistryCursor (
        block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (_other.data));
}

ResourceProvider::ThirdPartyRegistryCursor::ThirdPartyRegistryCursor (
    ResourceProvider::ThirdPartyRegistryCursor &&_other) noexcept
{
    new (&data) Original::ResourceProvider::ThirdPartyRegistryCursor (
        std::move (block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (_other.data)));
}

ResourceProvider::ThirdPartyRegistryCursor::~ThirdPartyRegistryCursor () noexcept
{
    block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (data).~ThirdPartyRegistryCursor ();
}

Memory::UniqueString ResourceProvider::ThirdPartyRegistryCursor::operator* () const noexcept
{
    return *block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (data);
}

ResourceProvider::ThirdPartyRegistryCursor &ResourceProvider::ThirdPartyRegistryCursor::operator++ () noexcept
{
    ++block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (data);
    return *this;
}

ResourceProvider::ThirdPartyRegistryCursor::ThirdPartyRegistryCursor (
    std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::ResourceProvider::ThirdPartyRegistryCursor (
        std::move (block_cast<Original::ResourceProvider::ThirdPartyRegistryCursor> (_data)));
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"ResourceProvider"}}};
    Original::ResourceProvider *resourceProvider = nullptr;
};

ResourceProvider::ResourceProvider (VirtualFileSystem::Context *_virtualFileSystemContext,
                                    Container::MappingRegistry _objectTypesRegistry,
                                    Container::MappingRegistry _patchableTypesRegistry) noexcept
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();

    internal.resourceProvider =
        new (internal.heap.Acquire (sizeof (Original::ResourceProvider), alignof (Original::ResourceProvider)))
            Original::ResourceProvider (_virtualFileSystemContext, std::move (_objectTypesRegistry),
                                        std::move (_patchableTypesRegistry));
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

SourceOperationResponse ResourceProvider::SaveSourceIndex (Memory::UniqueString _sourcePath,
                                                           const VirtualFileSystem::Entry &_output) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->SaveSourceIndex (_sourcePath, _output);
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
                                                                   std::uint64_t &_sizeOutput,
                                                                   std::uint8_t *&_dataOutput) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->LoadThirdPartyResource (_id, _allocator, _sizeOutput, _dataOutput);
}

ResourceProvider::ObjectRegistryCursor ResourceProvider::FindObjectsByType (
    const StandardLayout::Mapping &_type) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    Original::ResourceProvider::ObjectRegistryCursor cursor = internal.resourceProvider->FindObjectsByType (_type);
    return ObjectRegistryCursor (array_cast (cursor));
}

ResourceProvider::ThirdPartyRegistryCursor ResourceProvider::VisitAllThirdParty () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    Original::ResourceProvider::ThirdPartyRegistryCursor cursor = internal.resourceProvider->VisitAllThirdParty ();
    return ThirdPartyRegistryCursor (array_cast (cursor));
}

ObjectFormat ResourceProvider::GetObjectFormat (const StandardLayout::Mapping &_type,
                                                Memory::UniqueString _id) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->GetObjectFormat (_type, _id);
}

VirtualFileSystem::Entry ResourceProvider::GetObjectEntry (const StandardLayout::Mapping &_type,
                                                           Memory::UniqueString _id) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->GetObjectEntry (_type, _id);
}

VirtualFileSystem::Entry ResourceProvider::GetThirdPartyEntry (Memory::UniqueString _id) const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.resourceProvider);
    return internal.resourceProvider->GetThirdPartyEntry (_id);
}
} // namespace Emergence::Resource::Provider
