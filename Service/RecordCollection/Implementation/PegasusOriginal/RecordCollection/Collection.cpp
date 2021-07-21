#include <cassert>

#include <Pegasus/Storage.hpp>

#include <RecordCollection/Collection.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
Collection::Allocator::Allocator (Collection::Allocator &&_other) noexcept
{
    new (&data) Pegasus::Storage::Allocator (std::move (block_cast <Pegasus::Storage::Allocator> (_other.data)));
}

Collection::Allocator::~Allocator () noexcept
{
    block_cast <Pegasus::Storage::Allocator> (data).~Allocator ();
}

void *Collection::Allocator::Allocate () noexcept
{
    return block_cast <Pegasus::Storage::Allocator> (data).Next ();
}

Collection::Allocator::Allocator (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::Storage::Allocator (std::move (block_cast <Pegasus::Storage::Allocator> (*_data)));
}

Collection::LinearRepresentationIterator::LinearRepresentationIterator (
    const Collection::LinearRepresentationIterator &_other) noexcept
{
    new (&data) Pegasus::Storage::OrderedIndexIterator (
        block_cast <Pegasus::Storage::OrderedIndexIterator> (_other.data));
}

Collection::LinearRepresentationIterator::LinearRepresentationIterator (
    Collection::LinearRepresentationIterator &&_other) noexcept
{
    new (&data) Pegasus::Storage::OrderedIndexIterator (
        std::move (block_cast <Pegasus::Storage::OrderedIndexIterator> (_other.data)));
}

Collection::LinearRepresentationIterator::~LinearRepresentationIterator () noexcept
{
    block_cast <Pegasus::Storage::OrderedIndexIterator> (data).~OrderedIndexIterator ();
}

LinearRepresentation Collection::LinearRepresentationIterator::operator * () const noexcept
{
    return LinearRepresentation ((*block_cast <Pegasus::Storage::OrderedIndexIterator> (data)).Get ());
}

Collection::LinearRepresentationIterator &Collection::LinearRepresentationIterator::operator ++ () noexcept
{
    ++block_cast <Pegasus::Storage::OrderedIndexIterator> (data);
    return *this;
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationIterator::operator ++ (int) noexcept
{
    Pegasus::Storage::OrderedIndexIterator previous = block_cast <Pegasus::Storage::OrderedIndexIterator> (data)++;
    return LinearRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

Collection::LinearRepresentationIterator &Collection::LinearRepresentationIterator::operator -- () noexcept
{
    --block_cast <Pegasus::Storage::OrderedIndexIterator> (data);
    return *this;
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationIterator::operator -- (int) noexcept
{
    Pegasus::Storage::OrderedIndexIterator previous = block_cast <Pegasus::Storage::OrderedIndexIterator> (data)--;
    return LinearRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

bool Collection::LinearRepresentationIterator::operator == (
    const Collection::LinearRepresentationIterator &_other) const noexcept
{
    return block_cast <Pegasus::Storage::OrderedIndexIterator> (data) ==
           block_cast <Pegasus::Storage::OrderedIndexIterator> (_other.data);
}

bool Collection::LinearRepresentationIterator::operator != (
    const Collection::LinearRepresentationIterator &_other) const noexcept
{
    return !(*this == _other);
}

Collection::LinearRepresentationIterator &Collection::LinearRepresentationIterator::operator = (
    const LinearRepresentationIterator &_other) noexcept
{
    if (this != &_other)
    {
        this->~LinearRepresentationIterator ();
        new (this) LinearRepresentationIterator (_other);
    }

    return *this;
}

Collection::LinearRepresentationIterator &Collection::LinearRepresentationIterator::operator = (
    LinearRepresentationIterator &&_other) noexcept
{
    if (this != &_other)
    {
        this->~LinearRepresentationIterator ();
        new (this) LinearRepresentationIterator (std::move (_other));
    }

    return *this;
}

Collection::LinearRepresentationIterator::LinearRepresentationIterator (
    const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::Storage::OrderedIndexIterator (block_cast <Pegasus::Storage::OrderedIndexIterator> (*_data));
}

Collection::PointRepresentationIterator::PointRepresentationIterator (
    const Collection::PointRepresentationIterator &_other) noexcept
{
    new (&data) Pegasus::Storage::HashIndexIterator (
        block_cast <Pegasus::Storage::HashIndexIterator> (_other.data));
}

Collection::PointRepresentationIterator::PointRepresentationIterator (
    Collection::PointRepresentationIterator &&_other) noexcept
{
    new (&data) Pegasus::Storage::HashIndexIterator (
        std::move (block_cast <Pegasus::Storage::HashIndexIterator> (_other.data)));
}

Collection::PointRepresentationIterator::~PointRepresentationIterator () noexcept
{
    block_cast <Pegasus::Storage::HashIndexIterator> (data).~HashIndexIterator ();
}

PointRepresentation Collection::PointRepresentationIterator::operator * () const noexcept
{
    return PointRepresentation ((*block_cast <Pegasus::Storage::HashIndexIterator> (data)).Get ());
}

Collection::PointRepresentationIterator &Collection::PointRepresentationIterator::operator ++ () noexcept
{
    ++block_cast <Pegasus::Storage::HashIndexIterator> (data);
    return *this;
}

Collection::PointRepresentationIterator Collection::PointRepresentationIterator::operator ++ (int) noexcept
{
    Pegasus::Storage::HashIndexIterator previous = block_cast <Pegasus::Storage::HashIndexIterator> (data)++;
    return PointRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

Collection::PointRepresentationIterator &Collection::PointRepresentationIterator::operator -- () noexcept
{
    --block_cast <Pegasus::Storage::HashIndexIterator> (data);
    return *this;
}

Collection::PointRepresentationIterator Collection::PointRepresentationIterator::operator -- (int) noexcept
{
    Pegasus::Storage::HashIndexIterator previous = block_cast <Pegasus::Storage::HashIndexIterator> (data)--;
    return PointRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

bool Collection::PointRepresentationIterator::operator == (
    const Collection::PointRepresentationIterator &_other) const noexcept
{
    return block_cast <Pegasus::Storage::HashIndexIterator> (data) ==
           block_cast <Pegasus::Storage::HashIndexIterator> (_other.data);
}

bool Collection::PointRepresentationIterator::operator != (
    const Collection::PointRepresentationIterator &_other) const noexcept
{
    return !(*this == _other);
}

Collection::PointRepresentationIterator &Collection::PointRepresentationIterator::operator = (
    const PointRepresentationIterator &_other) noexcept
{
    if (this != &_other)
    {
        this->~PointRepresentationIterator ();
        new (this) PointRepresentationIterator (_other);
    }

    return *this;
}

Collection::PointRepresentationIterator &Collection::PointRepresentationIterator::operator = (
    PointRepresentationIterator &&_other) noexcept
{
    if (this != &_other)
    {
        this->~PointRepresentationIterator ();
        new (this) PointRepresentationIterator (std::move (_other));
    }

    return *this;
}

Collection::PointRepresentationIterator::PointRepresentationIterator (
    const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::Storage::HashIndexIterator (block_cast <Pegasus::Storage::HashIndexIterator> (*_data));
}

Collection::VolumetricRepresentationIterator::VolumetricRepresentationIterator (
    const Collection::VolumetricRepresentationIterator &_other) noexcept
{
    new (&data) Pegasus::Storage::VolumetricIndexIterator (
        block_cast <Pegasus::Storage::VolumetricIndexIterator> (_other.data));
}

Collection::VolumetricRepresentationIterator::VolumetricRepresentationIterator (
    Collection::VolumetricRepresentationIterator &&_other) noexcept
{
    new (&data) Pegasus::Storage::VolumetricIndexIterator (
        std::move (block_cast <Pegasus::Storage::VolumetricIndexIterator> (_other.data)));
}

Collection::VolumetricRepresentationIterator::~VolumetricRepresentationIterator () noexcept
{
    block_cast <Pegasus::Storage::VolumetricIndexIterator> (data).~VolumetricIndexIterator ();
}

VolumetricRepresentation Collection::VolumetricRepresentationIterator::operator * () const noexcept
{
    return VolumetricRepresentation ((*block_cast <Pegasus::Storage::VolumetricIndexIterator> (data)).Get ());
}

Collection::VolumetricRepresentationIterator &Collection::VolumetricRepresentationIterator::operator ++ () noexcept
{
    ++block_cast <Pegasus::Storage::VolumetricIndexIterator> (data);
    return *this;
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationIterator::operator ++ (int) noexcept
{
    Pegasus::Storage::VolumetricIndexIterator
        previous = block_cast <Pegasus::Storage::VolumetricIndexIterator> (data)++;
    return VolumetricRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

Collection::VolumetricRepresentationIterator &Collection::VolumetricRepresentationIterator::operator -- () noexcept
{
    --block_cast <Pegasus::Storage::VolumetricIndexIterator> (data);
    return *this;
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationIterator::operator -- (int) noexcept
{
    Pegasus::Storage::VolumetricIndexIterator
        previous = block_cast <Pegasus::Storage::VolumetricIndexIterator> (data)--;
    return VolumetricRepresentationIterator (reinterpret_cast <decltype (data) *> (&previous));
}

bool Collection::VolumetricRepresentationIterator::operator == (
    const Collection::VolumetricRepresentationIterator &_other) const noexcept
{
    return block_cast <Pegasus::Storage::VolumetricIndexIterator> (data) ==
           block_cast <Pegasus::Storage::VolumetricIndexIterator> (_other.data);
}

bool Collection::VolumetricRepresentationIterator::operator != (
    const Collection::VolumetricRepresentationIterator &_other) const noexcept
{
    return !(*this == _other);
}

Collection::VolumetricRepresentationIterator &Collection::VolumetricRepresentationIterator::operator = (
    const VolumetricRepresentationIterator &_other) noexcept
{
    if (this != &_other)
    {
        this->~VolumetricRepresentationIterator ();
        new (this) VolumetricRepresentationIterator (_other);
    }

    return *this;
}

Collection::VolumetricRepresentationIterator &Collection::VolumetricRepresentationIterator::operator = (
    VolumetricRepresentationIterator &&_other) noexcept
{
    if (this != &_other)
    {
        this->~VolumetricRepresentationIterator ();
        new (this) VolumetricRepresentationIterator (std::move (_other));
    }

    return *this;
}

Collection::VolumetricRepresentationIterator::VolumetricRepresentationIterator (
    const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::Storage::VolumetricIndexIterator (
        block_cast <Pegasus::Storage::VolumetricIndexIterator> (*_data));
}

Collection::Collection (StandardLayout::Mapping _typeMapping)
    : handle (new Pegasus::Storage (std::move (_typeMapping)))
{
}

Collection::Collection (Collection &&_other) noexcept
    : handle (_other.handle)
{
    assert (handle);
    _other.handle = nullptr;
}

Collection::~Collection () noexcept
{
    if (handle)
    {
        // TODO: Think about better way to create collections than new/delete.
        delete static_cast <Pegasus::Storage *> (handle);
    }
}

Collection::Allocator Collection::AllocateAndInsert () noexcept
{
    assert (handle);
    Pegasus::Storage::Allocator allocator = static_cast <Pegasus::Storage *> (handle)->AllocateAndInsert ();
    return Allocator (reinterpret_cast <decltype (Allocator::data) *> (&allocator));
}

LinearRepresentation Collection::CreateLinearRepresentation (StandardLayout::FieldId _keyField) const noexcept
{
    assert (handle);
    Handling::Handle <Pegasus::OrderedIndex> index =
        static_cast <Pegasus::Storage *> (handle)->CreateOrderedIndex (_keyField);
    return LinearRepresentation (index.Get ());
}

PointRepresentation
Collection::CreatePointRepresentation (const std::vector <StandardLayout::FieldId> &_keyFields) const noexcept
{
    assert (handle);
    Handling::Handle <Pegasus::HashIndex> index =
        static_cast <Pegasus::Storage *> (handle)->CreateHashIndex (_keyFields);
    return PointRepresentation (index.Get ());
}

VolumetricRepresentation
Collection::CreateVolumetricRepresentation (const std::vector <DimensionDescriptor> &_dimensions) const noexcept
{
    assert (handle);
    // Volumetric representation creation is rare operation, therefore it's ok to dynamically allocate vector here.
    std::vector <Pegasus::VolumetricIndex::DimensionDescriptor> convertedDimensions;

    for (const DimensionDescriptor &dimension : _dimensions)
    {
        convertedDimensions.emplace_back (
            Pegasus::VolumetricIndex::DimensionDescriptor
                {
                    *reinterpret_cast <const Pegasus::VolumetricIndex::SupportedAxisValue *> (
                        dimension.globalMinBorder),
                    dimension.minBorderField,

                    *reinterpret_cast <const Pegasus::VolumetricIndex::SupportedAxisValue *> (
                        dimension.globalMaxBorder),
                    dimension.maxBorderField,
                }
        );
    }

    Handling::Handle <Pegasus::VolumetricIndex> index =
        static_cast <Pegasus::Storage *> (handle)->CreateVolumetricIndex (convertedDimensions);
    return VolumetricRepresentation (index.Get ());
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::OrderedIndexIterator iterator =
        static_cast <const Pegasus::Storage *> (handle)->BeginOrderedIndices ();

    return LinearRepresentationIterator (
        reinterpret_cast <decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::LinearRepresentationIterator Collection::LinearRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::OrderedIndexIterator iterator =
        static_cast <const Pegasus::Storage *> (handle)->EndOrderedIndices ();

    return LinearRepresentationIterator (
        reinterpret_cast <decltype (LinearRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::HashIndexIterator iterator = static_cast <const Pegasus::Storage *> (handle)->BeginHashIndices ();
    return PointRepresentationIterator (reinterpret_cast <decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::PointRepresentationIterator Collection::PointRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::HashIndexIterator iterator = static_cast <const Pegasus::Storage *> (handle)->EndHashIndices ();
    return PointRepresentationIterator (reinterpret_cast <decltype (PointRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationBegin () const noexcept
{
    assert (handle);
    Pegasus::Storage::VolumetricIndexIterator iterator =
        static_cast <const Pegasus::Storage *> (handle)->BeginVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast <decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

Collection::VolumetricRepresentationIterator Collection::VolumetricRepresentationEnd () const noexcept
{
    assert (handle);
    Pegasus::Storage::VolumetricIndexIterator iterator =
        static_cast <const Pegasus::Storage *> (handle)->EndVolumetricIndices ();

    return VolumetricRepresentationIterator (
        reinterpret_cast <decltype (VolumetricRepresentationIterator::data) *> (&iterator));
}

Collection &Collection::operator = (Collection &&_other) noexcept
{
    if (handle != _other.handle)
    {
        this->~Collection ();
        handle = _other.handle;
        _other.handle = nullptr;
    }

    return *this;
}
} // namespace Emergence::RecordCollection