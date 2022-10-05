#include <cassert>

#include <API/Common/Implementation/Cursor.hpp>
#include <API/Common/Implementation/Iterator.hpp>

#include <Handling/Handle.hpp>

#include <Pegasus/VolumetricIndex.hpp>

#include <RecordCollection/VolumetricRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
using ShapeIntersectionReadCursor = VolumetricRepresentation::ShapeIntersectionReadCursor;

using ShapeIntersectionReadCursorImplementation = Pegasus::VolumetricIndex::ShapeIntersectionReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (ShapeIntersectionReadCursor,
                                                      ShapeIntersectionReadCursorImplementation)

using ShapeIntersectionEditCursor = VolumetricRepresentation::ShapeIntersectionEditCursor;

using ShapeIntersectionEditCursorImplementation = Pegasus::VolumetricIndex::ShapeIntersectionEditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (ShapeIntersectionEditCursor,
                                                      ShapeIntersectionEditCursorImplementation)

using RayIntersectionReadCursor = VolumetricRepresentation::RayIntersectionReadCursor;

using RayIntersectionReadCursorImplementation = Pegasus::VolumetricIndex::RayIntersectionReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (RayIntersectionReadCursor,
                                                      RayIntersectionReadCursorImplementation)

using RayIntersectionEditCursor = VolumetricRepresentation::RayIntersectionEditCursor;

using RayIntersectionEditCursorImplementation = Pegasus::VolumetricIndex::RayIntersectionEditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (RayIntersectionEditCursor,
                                                      RayIntersectionEditCursorImplementation)

using DimensionIterator = VolumetricRepresentation::DimensionIterator;

using DimensionIteratorImplementation = Pegasus::VolumetricIndex::DimensionIterator;

// NOLINTNEXTLINE(modernize-use-auto): It's impossible to use auto there.
EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (DimensionIterator, DimensionIteratorImplementation)

VolumetricRepresentation::DimensionIterator::Dimension VolumetricRepresentation::DimensionIterator::operator* ()
    const noexcept
{
    const Pegasus::VolumetricIndex::Dimension &dimension = *block_cast<DimensionIteratorImplementation> (data);
    return {&dimension.min, dimension.minField, &dimension.max, dimension.maxField};
}

VolumetricRepresentation::VolumetricRepresentation (const VolumetricRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::VolumetricIndex> (
        *reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&_other.handle));
}

VolumetricRepresentation::VolumetricRepresentation (VolumetricRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::VolumetricIndex> (
        std::move (*reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&_other.handle)));
}

VolumetricRepresentation::~VolumetricRepresentation () noexcept
{
    if (handle)
    {
        reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->~Handle ();
    }
}

VolumetricRepresentation::ShapeIntersectionReadCursor VolumetricRepresentation::ReadShapeIntersections (
    VolumetricRepresentation::Shape _shape) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index = reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::ShapeIntersectionReadCursor cursor = index->LookupShapeIntersectionToRead (_shape);
    return ShapeIntersectionReadCursor (
        array_cast<decltype (cursor), ShapeIntersectionReadCursor::DATA_MAX_SIZE> (cursor));
}

VolumetricRepresentation::ShapeIntersectionEditCursor VolumetricRepresentation::EditShapeIntersections (
    VolumetricRepresentation::Shape _shape) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index = reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::ShapeIntersectionEditCursor cursor = index->LookupShapeIntersectionToEdit (_shape);
    return ShapeIntersectionEditCursor (
        array_cast<decltype (cursor), ShapeIntersectionEditCursor::DATA_MAX_SIZE> (cursor));
}

VolumetricRepresentation::RayIntersectionReadCursor VolumetricRepresentation::ReadRayIntersections (
    VolumetricRepresentation::Ray _ray, float _rayLength) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index = reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::RayIntersectionReadCursor cursor = index->LookupRayIntersectionToRead (_ray, _rayLength);
    return RayIntersectionReadCursor (array_cast<decltype (cursor), RayIntersectionReadCursor::DATA_MAX_SIZE> (cursor));
}

VolumetricRepresentation::RayIntersectionEditCursor VolumetricRepresentation::EditRayIntersections (
    VolumetricRepresentation::Ray _ray, float _rayLength) noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index = reinterpret_cast<Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    Pegasus::VolumetricIndex::RayIntersectionEditCursor cursor = index->LookupRayIntersectionToEdit (_ray, _rayLength);
    return RayIntersectionEditCursor (array_cast<decltype (cursor), RayIntersectionEditCursor::DATA_MAX_SIZE> (cursor));
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionBegin () const noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    auto iterator = index->BeginDimensions ();
    return DimensionIterator (array_cast (iterator));
}

VolumetricRepresentation::DimensionIterator VolumetricRepresentation::DimensionEnd () const noexcept
{
    assert (handle);
    Pegasus::VolumetricIndex *index =
        reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ();

    auto iterator = index->EndDimensions ();
    return DimensionIterator (array_cast (iterator));
}

const StandardLayout::Mapping &VolumetricRepresentation::GetTypeMapping () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&handle)->Get ()->GetRecordMapping ();
}

bool VolumetricRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&handle);
    Pegasus::VolumetricIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast<Handling::Handle<Pegasus::VolumetricIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast<Handling::Handle<Pegasus::VolumetricIndex> &> (realHandle) = index;

    return canBeDropped;
}

void VolumetricRepresentation::Drop () noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::VolumetricIndex> *> (&handle);
    Pegasus::VolumetricIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast<Handling::Handle<Pegasus::VolumetricIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool VolumetricRepresentation::operator== (const VolumetricRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

VolumetricRepresentation &VolumetricRepresentation::operator= (const VolumetricRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~VolumetricRepresentation ();
        new (this) VolumetricRepresentation (_other);
    }

    return *this;
}

VolumetricRepresentation &VolumetricRepresentation::operator= (VolumetricRepresentation &&_other) noexcept
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
    static_assert (sizeof (handle) == sizeof (Handling::Handle<Pegasus::VolumetricIndex>));
    new (&handle) Handling::Handle<Pegasus::VolumetricIndex> (static_cast<Pegasus::VolumetricIndex *> (_handle));
}
} // namespace Emergence::RecordCollection
