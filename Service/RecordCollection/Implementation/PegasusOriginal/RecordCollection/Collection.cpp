#include <cassert>

#include <API/Common/Implementation/Iterator.hpp>

#include <Pegasus/Storage.hpp>

#include <RecordCollection/Collection.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence
{
namespace RecordCollection
{
Collection::Allocator::Allocator (Collection::Allocator &&_other) noexcept
{
    new (&data) Pegasus::Storage::Allocator (std::move (block_cast<Pegasus::Storage::Allocator> (_other.data)));
}

Collection::Allocator::~Allocator () noexcept
{
    block_cast<Pegasus::Storage::Allocator> (data).~Allocator ();
}

void *Collection::Allocator::Allocate () noexcept
{
    return block_cast<Pegasus::Storage::Allocator> (data).Next ();
}

Collection::Allocator::Allocator (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::Storage::Allocator (std::move (block_cast<Pegasus::Storage::Allocator> (*_data)));
}

using LinearRepresentationIterator = Collection::LinearRepresentationIterator;

using LinearRepresentationIteratorImplementation = Pegasus::Storage::OrderedIndexIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (LinearRepresentationIterator,
                                                                 LinearRepresentationIteratorImplementation)

LinearRepresentation Collection::LinearRepresentationIterator::operator* () const noexcept
{
    return LinearRepresentation ((*block_cast<Pegasus::Storage::OrderedIndexIterator> (data)).Get ());
}

using PointRepresentationIterator = Collection::PointRepresentationIterator;

using PointRepresentationIteratorImplementation = Pegasus::Storage::HashIndexIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (PointRepresentationIterator,
                                                                 PointRepresentationIteratorImplementation)

PointRepresentation Collection::PointRepresentationIterator::operator* () const noexcept
{
    return PointRepresentation ((*block_cast<Pegasus::Storage::HashIndexIterator> (data)).Get ());
}

using VolumetricRepresentationIterator = Collection::VolumetricRepresentationIterator;

using VolumetricRepresentationIteratorImplementation = Pegasus::Storage::VolumetricIndexIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (VolumetricRepresentationIterator,
                                                                 VolumetricRepresentationIteratorImplementation)

VolumetricRepresentation Collection::VolumetricRepresentationIterator::operator* () const noexcept
{
    return VolumetricRepresentation ((*block_cast<Pegasus::Storage::VolumetricIndexIterator> (data)).Get ());
}

struct InternalData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"Collection"}}};
    Pegasus::Storage *storage = nullptr;
};

Collection::Collection (StandardLayout::Mapping _typeMapping)
{
    auto &internal = *new (&data) InternalData ();
    auto placeholder = internal.heap.GetAllocationGroup ().PlaceOnTop ();
    internal.storage =
        new (internal.heap.Acquire (sizeof (Pegasus::Storage))) Pegasus::Storage (std::move (_typeMapping));
}

Collection::Collection (Collection &&_other) noexcept
{
    auto &internal = *new (&data) InternalData ();
    internal.storage = block_cast<InternalData> (_other.data).storage;
    assert (internal.storage);
    block_cast<InternalData> (_other.data).storage = nullptr;
}

Collection::~Collection () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    if (internal.storage)
    {
        internal.storage->~Storage ();
        internal.heap.Release (internal.storage, sizeof (Pegasus::Storage));
    }

    internal.~InternalData ();
}

Collection::Allocator Collection::AllocateAndInsert () noexcept
{
    auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::Allocator allocator = internal.storage->AllocateAndInsert ();
    return Allocator (reinterpret_cast<decltype (Allocator::data) *> (&allocator));
}

LinearRepresentation Collection::CreateLinearRepresentation (StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Handling::Handle<Pegasus::OrderedIndex> index = internal.storage->CreateOrderedIndex (_keyField);
    return LinearRepresentation (index.Get ());
}

PointRepresentation Collection::CreatePointRepresentation (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Handling::Handle<Pegasus::HashIndex> index = internal.storage->CreateHashIndex (_keyFields);
    return PointRepresentation (index.Get ());
}

VolumetricRepresentation Collection::CreateVolumetricRepresentation (
    const Container::Vector<DimensionDescriptor> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    // Volumetric representation creation is rare operation, therefore it's ok to dynamically allocate vector here.
    Container::Vector<Pegasus::VolumetricIndex::DimensionDescriptor> convertedDimensions {_dimensions.get_allocator ()};
    convertedDimensions.reserve (_dimensions.size ());

    for (const DimensionDescriptor &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (Pegasus::VolumetricIndex::DimensionDescriptor {
            *reinterpret_cast<const Pegasus::VolumetricIndex::SupportedAxisValue *> (dimension.globalMinBorder),
            dimension.minBorderField,

            *reinterpret_cast<const Pegasus::VolumetricIndex::SupportedAxisValue *> (dimension.globalMaxBorder),
            dimension.maxBorderField,
        });
    }

    Handling::Handle<Pegasus::VolumetricIndex> index = internal.storage->CreateVolumetricIndex (convertedDimensions);
    return VolumetricRepresentation (index.Get ());
}

const StandardLayout::Mapping &Collection::GetTypeMapping () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    return internal.storage->GetRecordMapping ();
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::OrderedIndexIterator iterator = internal.storage->BeginOrderedIndices ();

    return LinearRepresentationIterator (reinterpret_cast<decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::OrderedIndexIterator iterator = internal.storage->EndOrderedIndices ();

    return LinearRepresentationIterator (reinterpret_cast<decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::HashIndexIterator iterator = internal.storage->BeginHashIndices ();
    return PointRepresentationIterator (reinterpret_cast<decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::HashIndexIterator iterator = internal.storage->EndHashIndices ();
    return PointRepresentationIterator (reinterpret_cast<decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::VolumetricIndexIterator iterator = internal.storage->BeginVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast<decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    Pegasus::Storage::VolumetricIndexIterator iterator = internal.storage->EndVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast<decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

void Collection::SetUnsafeReadAllowed (bool _allowed) noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    assert (internal.storage);
    internal.storage->SetUnsafeReadAllowed (_allowed);
}

Collection &Collection::operator= (Collection &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Collection ();
        new (this) Collection (std::move (_other));
    }

    return *this;
}
} // namespace RecordCollection

namespace Memory
{
using namespace Literals;

Profiler::AllocationGroup DefaultAllocationGroup<RecordCollection::Collection::DimensionDescriptor>::Get () noexcept
{
    return Profiler::AllocationGroup {"DimensionConfiguration"_us};
}
} // namespace Memory
} // namespace Emergence
