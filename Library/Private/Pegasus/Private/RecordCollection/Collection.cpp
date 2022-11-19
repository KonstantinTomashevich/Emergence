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

Collection::Allocator::Allocator (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Pegasus::Storage::Allocator (std::move (block_cast<Pegasus::Storage::Allocator> (_data)));
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

using SignalRepresentationIterator = Collection::SignalRepresentationIterator;

using SignalRepresentationIteratorImplementation = Pegasus::Storage::SignalIndexIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (SignalRepresentationIterator,
                                                                 SignalRepresentationIteratorImplementation)

SignalRepresentation Collection::SignalRepresentationIterator::operator* () const noexcept
{
    return SignalRepresentation ((*block_cast<Pegasus::Storage::SignalIndexIterator> (data)).Get ());
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
    internal.storage = new (internal.heap.Acquire (sizeof (Pegasus::Storage), alignof (Pegasus::Storage)))
        Pegasus::Storage (std::move (_typeMapping));
}

Collection::Collection (Collection &&_other) noexcept
{
    auto &internal = *new (&data) InternalData ();
    internal.storage = block_cast<InternalData> (_other.data).storage;
    EMERGENCE_ASSERT (internal.storage);
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
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::Allocator allocator = internal.storage->AllocateAndInsert ();
    return Allocator (array_cast (allocator));
}

LinearRepresentation Collection::CreateLinearRepresentation (StandardLayout::FieldId _keyField) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Handling::Handle<Pegasus::OrderedIndex> index = internal.storage->CreateOrderedIndex (_keyField);
    return LinearRepresentation (index.Get ());
}

PointRepresentation Collection::CreatePointRepresentation (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Handling::Handle<Pegasus::HashIndex> index = internal.storage->CreateHashIndex (_keyFields);
    return PointRepresentation (index.Get ());
}

SignalRepresentation Collection::CreateSignalRepresentation (
    StandardLayout::FieldId _keyField, const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Handling::Handle<Pegasus::SignalIndex> index = internal.storage->CreateSignalIndex (_keyField, _signaledValue);
    return SignalRepresentation (index.Get ());
}

VolumetricRepresentation Collection::CreateVolumetricRepresentation (
    const Container::Vector<DimensionDescriptor> &_dimensions) noexcept
{
    auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    // Volumetric representation creation is rare operation, therefore it's ok to dynamically allocate vector here.
    Container::Vector<Pegasus::VolumetricIndex::DimensionDescriptor> convertedDimensions {_dimensions.get_allocator ()};
    convertedDimensions.reserve (_dimensions.size ());

    for (const DimensionDescriptor &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (Pegasus::VolumetricIndex::DimensionDescriptor {
            dimension.minBorderField,
            *reinterpret_cast<const Pegasus::VolumetricIndex::ValuePlaceholder *> (dimension.globalMinBorder),
            dimension.maxBorderField,
            *reinterpret_cast<const Pegasus::VolumetricIndex::ValuePlaceholder *> (dimension.globalMaxBorder),
        });
    }

    Handling::Handle<Pegasus::VolumetricIndex> index = internal.storage->CreateVolumetricIndex (convertedDimensions);
    return VolumetricRepresentation (index.Get ());
}

const StandardLayout::Mapping &Collection::GetTypeMapping () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    return internal.storage->GetRecordMapping ();
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::OrderedIndexIterator iterator = internal.storage->BeginOrderedIndices ();

    return LinearRepresentationIterator (array_cast (iterator));
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::OrderedIndexIterator iterator = internal.storage->EndOrderedIndices ();

    return LinearRepresentationIterator (array_cast (iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::HashIndexIterator iterator = internal.storage->BeginHashIndices ();
    return PointRepresentationIterator (array_cast (iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::HashIndexIterator iterator = internal.storage->EndHashIndices ();
    return PointRepresentationIterator (array_cast (iterator));
}

Collection::SignalRepresentationIterator Collection::SignalRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::SignalIndexIterator iterator = internal.storage->BeginSignalIndices ();
    return SignalRepresentationIterator (array_cast (iterator));
}

Collection::SignalRepresentationIterator Collection::SignalRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::SignalIndexIterator iterator = internal.storage->EndSignalIndices ();
    return SignalRepresentationIterator (array_cast (iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationBegin () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::VolumetricIndexIterator iterator = internal.storage->BeginVolumetricIndices ();

    return VolumetricRepresentationIterator (array_cast (iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationEnd () const noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
    Pegasus::Storage::VolumetricIndexIterator iterator = internal.storage->EndVolumetricIndices ();

    return VolumetricRepresentationIterator (array_cast (iterator));
}

void Collection::SetUnsafeReadAllowed (bool _allowed) noexcept
{
    const auto &internal = block_cast<InternalData> (data);
    EMERGENCE_ASSERT (internal.storage);
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
