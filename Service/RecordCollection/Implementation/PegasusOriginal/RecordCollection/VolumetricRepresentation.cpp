#include <cassert>

#include <Handling/Handle.hpp>

#include <Pegasus/VolumetricIndex.hpp>

#include <RecordCollection/VolumetricRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
VolumetricRepresentation::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    const VolumetricRepresentation::ShapeIntersectionReadCursor &_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::ShapeIntersectionReadCursor (
        block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (_other.data));
}

VolumetricRepresentation::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricRepresentation::ShapeIntersectionReadCursor &&_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::ShapeIntersectionReadCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (_other.data)));
}

VolumetricRepresentation::ShapeIntersectionReadCursor::~ShapeIntersectionReadCursor () noexcept
{
    block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (data).~ShapeIntersectionReadCursor ();
}

const void *VolumetricRepresentation::ShapeIntersectionReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (data);
}

VolumetricRepresentation::ShapeIntersectionReadCursor &
VolumetricRepresentation::ShapeIntersectionReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (data);
    return *this;
}

VolumetricRepresentation::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::VolumetricIndex::ShapeIntersectionReadCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::ShapeIntersectionReadCursor> (*_data)));
}

VolumetricRepresentation::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricRepresentation::ShapeIntersectionEditCursor &&_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::ShapeIntersectionEditCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (_other.data)));
}

VolumetricRepresentation::ShapeIntersectionEditCursor::~ShapeIntersectionEditCursor () noexcept
{
    block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (data).~ShapeIntersectionEditCursor ();
}

void *VolumetricRepresentation::ShapeIntersectionEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (data);
}

VolumetricRepresentation::ShapeIntersectionEditCursor &
VolumetricRepresentation::ShapeIntersectionEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (data);
    return *this;
}

VolumetricRepresentation::ShapeIntersectionEditCursor &
VolumetricRepresentation::ShapeIntersectionEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (data);
    return *this;
}

VolumetricRepresentation::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::VolumetricIndex::ShapeIntersectionEditCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::ShapeIntersectionEditCursor> (*_data)));
}

VolumetricRepresentation::RayIntersectionReadCursor::RayIntersectionReadCursor (
    const VolumetricRepresentation::RayIntersectionReadCursor &_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::RayIntersectionReadCursor (
        block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (_other.data));
}

VolumetricRepresentation::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricRepresentation::RayIntersectionReadCursor &&_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::RayIntersectionReadCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (_other.data)));
}

VolumetricRepresentation::RayIntersectionReadCursor::~RayIntersectionReadCursor () noexcept
{
    block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (data).~RayIntersectionReadCursor ();
}

const void *VolumetricRepresentation::RayIntersectionReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (data);
}

VolumetricRepresentation::RayIntersectionReadCursor &
VolumetricRepresentation::RayIntersectionReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (data);
    return *this;
}

VolumetricRepresentation::RayIntersectionReadCursor::RayIntersectionReadCursor (
    std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::VolumetricIndex::RayIntersectionReadCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::RayIntersectionReadCursor> (*_data)));
}

VolumetricRepresentation::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricRepresentation::RayIntersectionEditCursor &&_other) noexcept
{
    new (&data) Pegasus::VolumetricIndex::RayIntersectionEditCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (_other.data)));
}

VolumetricRepresentation::RayIntersectionEditCursor::~RayIntersectionEditCursor () noexcept
{
    block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (data).~RayIntersectionEditCursor ();
}

void *VolumetricRepresentation::RayIntersectionEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (data);
}

VolumetricRepresentation::RayIntersectionEditCursor &
VolumetricRepresentation::RayIntersectionEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (data);
    return *this;
}

VolumetricRepresentation::RayIntersectionEditCursor &
VolumetricRepresentation::RayIntersectionEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (data);
    return *this;
}

VolumetricRepresentation::RayIntersectionEditCursor::RayIntersectionEditCursor (
    std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::VolumetricIndex::RayIntersectionEditCursor (
        std::move (block_cast <Pegasus::VolumetricIndex::RayIntersectionEditCursor> (*_data)));
}

using DimensionIteratorBaseType = InplaceVector <
    Pegasus::VolumetricIndex::Dimension, Pegasus::Constants::VolumetricIndex::MAX_DIMENSIONS>::ConstIterator;

VolumetricRepresentation::DimensionIterator::DimensionIterator (
    const VolumetricRepresentation::DimensionIterator &_other) noexcept
{
    new (&data) DimensionIteratorBaseType (block_cast <DimensionIteratorBaseType> (_other.data));
}

VolumetricRepresentation::DimensionIterator::DimensionIterator (
    VolumetricRepresentation::DimensionIterator &&_other) noexcept
{
    new (&data) DimensionIteratorBaseType (std::move (block_cast <DimensionIteratorBaseType> (_other.data)));
}

VolumetricRepresentation::DimensionIterator::~DimensionIterator () noexcept
{
    block_cast <DimensionIteratorBaseType> (data).~DimensionIteratorBaseType ();
}

VolumetricRepresentation::DimensionIterator::Dimension
VolumetricRepresentation::DimensionIterator::operator * () const noexcept
{
    const Pegasus::VolumetricIndex::Dimension &dimension = *block_cast <DimensionIteratorBaseType> (data);
    return {&dimension.globalMinBorder, dimension.minBorderField, &dimension.globalMaxBorder, dimension.maxBorderField};
}

VolumetricRepresentation::DimensionIterator &VolumetricRepresentation::DimensionIterator::operator ++ () noexcept
{
    ++block_cast <DimensionIteratorBaseType> (data);
    return *this;
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionIterator::operator ++ (int) noexcept
{
    auto previous = block_cast <DimensionIteratorBaseType> (data)++;
    return DimensionIterator (reinterpret_cast <decltype (data) *> (&previous));
}

VolumetricRepresentation::DimensionIterator &VolumetricRepresentation::DimensionIterator::operator -- () noexcept
{
    --block_cast <DimensionIteratorBaseType> (data);
    return *this;
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionIterator::operator -- (int) noexcept
{
    auto previous = block_cast <DimensionIteratorBaseType> (data)--;
    return DimensionIterator (reinterpret_cast <decltype (data) *> (&previous));
}

bool VolumetricRepresentation::DimensionIterator::operator == (
    const VolumetricRepresentation::DimensionIterator &_other) const noexcept
{
    return block_cast <DimensionIteratorBaseType> (data) == block_cast <DimensionIteratorBaseType> (_other.data);
}

bool VolumetricRepresentation::DimensionIterator::operator != (
    const VolumetricRepresentation::DimensionIterator &_other) const noexcept
{
    return !(*this == _other);
}

VolumetricRepresentation::DimensionIterator &VolumetricRepresentation::DimensionIterator::operator = (
    const VolumetricRepresentation::DimensionIterator &_other) noexcept
{
    if (this != &_other)
    {
        this->~DimensionIterator ();
        new (this) DimensionIterator (_other);
    }

    return *this;
}

VolumetricRepresentation::DimensionIterator &VolumetricRepresentation::DimensionIterator::operator = (
    VolumetricRepresentation::DimensionIterator &&_other) noexcept
{
    if (this != &_other)
    {
        this->~DimensionIterator ();
        new (this) DimensionIterator (std::move (_other));
    }

    return *this;
}

VolumetricRepresentation::DimensionIterator::DimensionIterator (
    const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) DimensionIteratorBaseType (std::move (block_cast <DimensionIteratorBaseType> (*_data)));
}

VolumetricRepresentation::VolumetricRepresentation (const VolumetricRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle <Pegasus::VolumetricIndex> (
        *reinterpret_cast <const Handling::Handle <Pegasus::VolumetricIndex> *> (&_other.handle));
}

VolumetricRepresentation::VolumetricRepresentation (VolumetricRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle <Pegasus::VolumetricIndex> (
        std::move (*reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&_other.handle)));
}

VolumetricRepresentation::~VolumetricRepresentation () noexcept
{
    if (handle)
    {
        reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->~Handle ();
    }
}

VolumetricRepresentation::ShapeIntersectionReadCursor VolumetricRepresentation::ReadShapeIntersections (
    VolumetricRepresentation::Shape _shape) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::ShapeIntersectionReadCursor cursor = index->LookupShapeIntersectionToRead (
        *static_cast <const Pegasus::VolumetricIndex::AxisAlignedShapeContainer *> (_shape));
    return ShapeIntersectionReadCursor (reinterpret_cast <decltype (ShapeIntersectionReadCursor::data) *> (&cursor));
}

VolumetricRepresentation::ShapeIntersectionEditCursor VolumetricRepresentation::EditShapeIntersections (
    VolumetricRepresentation::Shape _shape) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::ShapeIntersectionEditCursor cursor = index->LookupShapeIntersectionToEdit (
        *static_cast <const Pegasus::VolumetricIndex::AxisAlignedShapeContainer *> (_shape));
    return ShapeIntersectionEditCursor (reinterpret_cast <decltype (ShapeIntersectionEditCursor::data) *> (&cursor));
}

VolumetricRepresentation::RayIntersectionReadCursor VolumetricRepresentation::ReadRayIntersections (
    VolumetricRepresentation::Ray _ray, float _rayLength) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::RayIntersectionReadCursor cursor = index->LookupRayIntersectionToRead (
        *static_cast <const Pegasus::VolumetricIndex::RayContainer *> (_ray), _rayLength);
    return RayIntersectionReadCursor (reinterpret_cast <decltype (RayIntersectionReadCursor::data) *> (&cursor));
}

VolumetricRepresentation::RayIntersectionEditCursor VolumetricRepresentation::EditRayIntersections (
    VolumetricRepresentation::Ray _ray, float _rayLength) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::RayIntersectionEditCursor cursor = index->LookupRayIntersectionToEdit (
        *static_cast <const Pegasus::VolumetricIndex::RayContainer *> (_ray), _rayLength);
    return RayIntersectionEditCursor (reinterpret_cast <decltype (RayIntersectionEditCursor::data) *> (&cursor));
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionBegin () const noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <const Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    auto iterator = index->GetDimensions ().Begin ();
    return DimensionIterator (reinterpret_cast <decltype (DimensionIterator::data) *> (&iterator));
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionEnd () const noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast <const Handling::Handle <Pegasus::VolumetricIndex> *> (&handle)->Get ();

    auto iterator = index->GetDimensions ().End ();
    return DimensionIterator (reinterpret_cast <decltype (DimensionIterator::data) *> (&iterator));
}

bool VolumetricRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast <const Handling::Handle <Pegasus::VolumetricIndex> *> (&handle);
    Pegasus::VolumetricIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast <Handling::Handle <Pegasus::VolumetricIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast <Handling::Handle <Pegasus::VolumetricIndex> &> (realHandle) = index;

    return canBeDropped;
}

void VolumetricRepresentation::Drop () noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast <const Handling::Handle <Pegasus::VolumetricIndex> *> (&handle);
    Pegasus::VolumetricIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast <Handling::Handle <Pegasus::VolumetricIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool VolumetricRepresentation::operator == (const VolumetricRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

VolumetricRepresentation &VolumetricRepresentation::operator = (const VolumetricRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~VolumetricRepresentation ();
        new (this) VolumetricRepresentation (_other);
    }

    return *this;
}

VolumetricRepresentation &VolumetricRepresentation::operator = (VolumetricRepresentation &&_other) noexcept
{
    if (this != &_other)
    {
        this->~VolumetricRepresentation ();
        new (this) VolumetricRepresentation (std::move (_other));
    }

    return *this;
}

VolumetricRepresentation::VolumetricRepresentation (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (Handling::Handle <Pegasus::VolumetricIndex>));
    new (&handle) Handling::Handle <Pegasus::VolumetricIndex> (static_cast <Pegasus::VolumetricIndex *> (_handle));
}
} // namespace Emergence::RecordCollection