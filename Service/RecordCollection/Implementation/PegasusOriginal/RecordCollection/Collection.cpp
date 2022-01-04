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

Collection::Collection (StandardLayout::Mapping _typeMapping) : handle (new Pegasus::Storage (std::move (_typeMapping)))
{
}

Collection::Collection (Collection &&_other) noexcept : handle (_other.handle)
{
    assert (handle);
    _other.handle = nullptr;
}

Collection::~Collection () noexcept
{
    delete static_cast<Pegasus::Storage *> (handle);
}

Collection::Allocator Collection::AllocateAndInsert () noexcept
{
    assert (handle);
    Pegasus::Storage::Allocator allocator = static_cast<Pegasus::Storage *> (handle)->AllocateAndInsert ();
    return Allocator (reinterpret_cast<decltype (Allocator::data) *> (&allocator));
}

LinearRepresentation Collection::CreateLinearRepresentation (StandardLayout::FieldId _keyField) noexcept
{
    assert (handle);
    Handling::Handle<Pegasus::OrderedIndex> index =
        static_cast<Pegasus::Storage *> (handle)->CreateOrderedIndex (_keyField);
    return LinearRepresentation (index.Get ());
}

PointRepresentation Collection::CreatePointRepresentation (
    const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept
{
    assert (handle);
    Handling::Handle<Pegasus::HashIndex> index = static_cast<Pegasus::Storage *> (handle)->CreateHashIndex (_keyFields);
    return PointRepresentation (index.Get ());
}

VolumetricRepresentation Collection::CreateVolumetricRepresentation (
    const Container::Vector<DimensionDescriptor> &_dimensions) noexcept
{
    assert (handle);
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

    Handling::Handle<Pegasus::VolumetricIndex> index =
        static_cast<Pegasus::Storage *> (handle)->CreateVolumetricIndex (convertedDimensions);
    return VolumetricRepresentation (index.Get ());
}

const StandardLayout::Mapping &Collection::GetTypeMapping () const noexcept
{
    assert (handle);
    return static_cast<const Pegasus::Storage *> (handle)->GetRecordMapping ();
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::OrderedIndexIterator iterator =
        static_cast<const Pegasus::Storage *> (handle)->BeginOrderedIndices ();

    return LinearRepresentationIterator (reinterpret_cast<decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::OrderedIndexIterator iterator =
        static_cast<const Pegasus::Storage *> (handle)->EndOrderedIndices ();

    return LinearRepresentationIterator (reinterpret_cast<decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::HashIndexIterator iterator = static_cast<const Pegasus::Storage *> (handle)->BeginHashIndices ();
    return PointRepresentationIterator (reinterpret_cast<decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::HashIndexIterator iterator = static_cast<const Pegasus::Storage *> (handle)->EndHashIndices ();
    return PointRepresentationIterator (reinterpret_cast<decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::VolumetricIndexIterator iterator =
        static_cast<const Pegasus::Storage *> (handle)->BeginVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast<decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::VolumetricIndexIterator iterator =
        static_cast<const Pegasus::Storage *> (handle)->EndVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast<decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

Collection &Collection::operator= (Collection &&_other) noexcept
{
    if (handle != _other.handle)
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
