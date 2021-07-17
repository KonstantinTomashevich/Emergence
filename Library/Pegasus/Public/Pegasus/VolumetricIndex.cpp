#include <algorithm>
#include <cassert>

#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>
#include <Pegasus/VolumetricIndex.hpp>

namespace Emergence::Pegasus
{
constexpr std::size_t GetMaxLeafCoordinateOnAxis (std::size_t _dimensions)
{
    return std::size_t (1u) << (Constants::VolumetricIndex::LEVELS[_dimensions - 1u] - 1u);
}

template <typename Type>
struct TypeOperations final
{
    using ValueType = Type;

    int Compare (const VolumetricIndex::SupportedAxisValue &_left,
                 const VolumetricIndex::SupportedAxisValue &_right) const noexcept;

    int Compare (const void *_left, const void *_right) const noexcept;

    VolumetricIndex::SupportedAxisValue Subtract (
        const VolumetricIndex::SupportedAxisValue &_left,
        const VolumetricIndex::SupportedAxisValue &_right) const noexcept;

    VolumetricIndex::SupportedAxisValue Divide (
        const VolumetricIndex::SupportedAxisValue &_value,
        const VolumetricIndex::SupportedAxisValue &_divider) const noexcept;

    VolumetricIndex::SupportedAxisValue Divide (
        const VolumetricIndex::SupportedAxisValue &_value, std::size_t divider) const noexcept;

    std::size_t TruncateToSizeType (const VolumetricIndex::SupportedAxisValue &_value) const noexcept;

    float ToFloat (const VolumetricIndex::SupportedAxisValue &_value) const noexcept;

private:
    NumericValueComparator <Type> comparator {};
};

template <typename Cursor>
struct CursorCommons final
{
    static void MoveToNextRecord (Cursor &_cursor) noexcept;

    /// If cursor is not finished and as a result of record deletion, record edition or cursor construction
    /// ::currentRecordIndex is invalid, current record is already visited or does not intersect with cursor
    /// primitive, we should execute ::MoveToNextRecord.
    static void FixCurrentRecordIndex (Cursor &_cursor) noexcept;
};

template <typename Callback>
auto DoWithCorrectTypeOperations (const StandardLayout::Field &_field, const Callback &_callback)
{
    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::INT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (int8_t):
                    return _callback (TypeOperations <int8_t> {});

                case sizeof (int16_t):
                    return _callback (TypeOperations <int16_t> {});

                case sizeof (int32_t):
                    return _callback (TypeOperations <int32_t> {});

                case sizeof (int64_t):
                    return _callback (TypeOperations <int64_t> {});
            }

            break;
        }
        case StandardLayout::FieldArchetype::UINT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (uint8_t):
                    return _callback (TypeOperations <uint8_t> {});

                case sizeof (uint16_t):
                    return _callback (TypeOperations <uint16_t> {});

                case sizeof (uint32_t):
                    return _callback (TypeOperations <uint32_t> {});

                case sizeof (uint64_t):
                    return _callback (TypeOperations <uint64_t> {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::FLOAT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (float):
                    return _callback (TypeOperations <float> {});

                case sizeof (double):
                    return _callback (TypeOperations <double> {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::BIT:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::STRING:
        {
            break;
        }
    }

    assert (false);
    return _callback (TypeOperations <float> ());
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue () noexcept
    : uint64 (0u)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (int8_t _value) noexcept
    : int8 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (int16_t _value) noexcept
    : int16 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (int32_t _value) noexcept
    : int32 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (int64_t _value) noexcept
    : int64 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (uint8_t _value) noexcept
    : uint8 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (uint16_t _value) noexcept
    : uint16 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (uint32_t _value) noexcept
    : uint32 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (uint64_t _value) noexcept
    : uint64 (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (float _value) noexcept
    : floating (_value)
{
}

VolumetricIndex::SupportedAxisValue::SupportedAxisValue (double _value) noexcept
    : doubleFloating (_value)
{
}

VolumetricIndex::ShapeIntersectionCursorBase::ShapeIntersectionCursorBase (
    const VolumetricIndex::ShapeIntersectionCursorBase &_other) noexcept
    : index (_other.index),
      sector (_other.sector),
      shape (_other.shape),
      currentCoordinate (_other.currentCoordinate),
      currentRecordIndex (_other.currentRecordIndex),
      visitedRecords (_other.visitedRecords)
{
    assert (index);
    ++index->activeCursors;
}

VolumetricIndex::ShapeIntersectionCursorBase::ShapeIntersectionCursorBase (
    VolumetricIndex::ShapeIntersectionCursorBase &&_other) noexcept
    : index (_other.index),
      sector (_other.sector),
      shape (_other.shape),
      currentCoordinate (_other.currentCoordinate),
      currentRecordIndex (_other.currentRecordIndex),
      visitedRecords (std::move (_other.visitedRecords))
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::ShapeIntersectionCursorBase::~ShapeIntersectionCursorBase () noexcept
{
    if (index)
    {
        --index->activeCursors;
    }
}

VolumetricIndex::ShapeIntersectionCursorBase::ShapeIntersectionCursorBase (
    VolumetricIndex *_index, const VolumetricIndex::LeafSector &_sector,
    const AxisAlignedShapeContainer &_shape) noexcept
    : index (_index),
      sector (_sector),
      shape (_shape),
      currentCoordinate (sector.min),
      currentRecordIndex (0u),
      visitedRecords (index->nextRecordId, false)
{
    assert (index);
#ifndef NDEBUG
    std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (index->dimensions.GetCount ());

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        assert (_sector.min[dimensionIndex] <= sector.max[dimensionIndex]);
        assert (sector.max[dimensionIndex] < maxCoordinate);
    }
#endif

    ++index->activeCursors;
    FixCurrentRecordIndex ();
}

bool VolumetricIndex::ShapeIntersectionCursorBase::IsFinished () const noexcept
{
    assert (index);
    return index->AreEqual (currentCoordinate, sector.max) &&
           currentRecordIndex >= index->leaves[index->GetLeafIndex (currentCoordinate)].records.size ();
}

void VolumetricIndex::ShapeIntersectionCursorBase::MoveToNextRecord () noexcept
{
    CursorCommons <ShapeIntersectionCursorBase>::MoveToNextRecord (*this);
}

const void *VolumetricIndex::ShapeIntersectionCursorBase::GetRecord () const noexcept
{
    assert (index);
    // TODO: Check GetLeafIndex performance. Maybe it makes sense to cache index?
    const LeafData &leaf = index->leaves[index->GetLeafIndex (currentCoordinate)];
    assert (currentRecordIndex < leaf.records.size () || IsFinished ());
    return currentRecordIndex < leaf.records.size () ? leaf.records[currentRecordIndex].record : nullptr;
}

VolumetricIndex *VolumetricIndex::ShapeIntersectionCursorBase::GetIndex () const noexcept
{
    return index;
}

void VolumetricIndex::ShapeIntersectionCursorBase::FixCurrentRecordIndex () noexcept
{
    CursorCommons <ShapeIntersectionCursorBase>::FixCurrentRecordIndex (*this);
}

template <typename Operations>
bool VolumetricIndex::ShapeIntersectionCursorBase::MoveToNextCoordinate (const Operations &) noexcept
{
    if (!index->AreEqual (currentCoordinate, sector.max))
    {
        currentRecordIndex = 0u;
        currentCoordinate = index->NextInsideSector (sector, currentCoordinate);
        return true;
    }
    else
    {
        return false;
    }
}

template <typename Operations>
bool VolumetricIndex::ShapeIntersectionCursorBase::CheckIntersection (
    const void *_record, const Operations &_operations) const noexcept
{
    assert (index);
    const auto *lookupShape = reinterpret_cast <const AxisAlignedShape <typename Operations::ValueType> *> (&shape);

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        const Dimension &dimension = index->dimensions[dimensionIndex];
        if (_operations.Compare (&lookupShape->Max (dimensionIndex), dimension.minBorderField.GetValue (_record)) < 0 ||
            _operations.Compare (&lookupShape->Min (dimensionIndex), dimension.maxBorderField.GetValue (_record)) > 0)
        {
            return false;
        }
    }

    return true;
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    const VolumetricIndex::ShapeIntersectionReadCursor &_other) noexcept
    : ShapeIntersectionCursorBase (_other)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricIndex::ShapeIntersectionReadCursor &&_other) noexcept
    : ShapeIntersectionCursorBase (std::move (_other))
{
}

VolumetricIndex::ShapeIntersectionReadCursor::~ShapeIntersectionReadCursor () noexcept
{
    if (GetIndex ())
    {
        GetIndex ()->storage->UnregisterReader ();
    }
}

const void *VolumetricIndex::ShapeIntersectionReadCursor::operator * () const noexcept
{
    return GetRecord ();
}

VolumetricIndex::ShapeIntersectionReadCursor &VolumetricIndex::ShapeIntersectionReadCursor::operator ++ () noexcept
{
    MoveToNextRecord ();
    return *this;
}

VolumetricIndex::ShapeIntersectionReadCursor::ShapeIntersectionReadCursor (
    VolumetricIndex *_index, const VolumetricIndex::LeafSector &_sector,
    const VolumetricIndex::AxisAlignedShapeContainer &_shape) noexcept
    : ShapeIntersectionCursorBase (_index, _sector, _shape)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterReader ();
}

VolumetricIndex::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricIndex::ShapeIntersectionEditCursor &&_other) noexcept
    : ShapeIntersectionCursorBase (std::move (_other))
{
}

VolumetricIndex::ShapeIntersectionEditCursor::~ShapeIntersectionEditCursor () noexcept
{
    if (GetIndex ())
    {
        if (!IsFinished ())
        {
            // Record can be stored in many coordinates, therefore
            // deletion by cursor can not be optimized for owner index.
            GetIndex ()->storage->EndRecordEdition (GetRecord (), nullptr);
        }

        GetIndex ()->storage->UnregisterWriter ();
    }
}

void *VolumetricIndex::ShapeIntersectionEditCursor::operator * () noexcept
{
    return const_cast <void *> (GetRecord ());
}

VolumetricIndex::ShapeIntersectionEditCursor &VolumetricIndex::ShapeIntersectionEditCursor::operator ~ () noexcept
{
    assert (!IsFinished ());
    // Record can be stored in many coordinates, therefore deletion by cursor can not be optimized for owner index.
    GetIndex ()->storage->DeleteRecord (**this, nullptr);

    FixCurrentRecordIndex ();
    BeginRecordEdition ();
    return *this;
}

VolumetricIndex::ShapeIntersectionEditCursor &VolumetricIndex::ShapeIntersectionEditCursor::operator ++ () noexcept
{
    assert (!IsFinished ());
    // Record can be stored in many coordinates, therefore edition by cursor can not be optimized for owner index.
    GetIndex ()->storage->EndRecordEdition (GetRecord (), nullptr);
    FixCurrentRecordIndex ();
    BeginRecordEdition ();
    return *this;
}

VolumetricIndex::ShapeIntersectionEditCursor::ShapeIntersectionEditCursor (
    VolumetricIndex *_index, const VolumetricIndex::LeafSector &_sector,
    const VolumetricIndex::AxisAlignedShapeContainer &_shape) noexcept
    : ShapeIntersectionCursorBase (_index, _sector, _shape)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void VolumetricIndex::ShapeIntersectionEditCursor::BeginRecordEdition () const noexcept
{
    assert (GetIndex ());
    if (!IsFinished ())
    {
        GetIndex ()->storage->BeginRecordEdition (GetRecord ());
    }
}

VolumetricIndex::RayIntersectionCursorBase::RayIntersectionCursorBase (
    const VolumetricIndex::RayIntersectionCursorBase &_other) noexcept
    : index (_other.index),
      currentPoint (_other.currentPoint),
      direction (_other.direction),
      distanceTraveled (_other.distanceTraveled),
      ray (_other.ray),
      maxDistance (_other.maxDistance),
      currentCoordinate (_other.currentCoordinate),
      currentRecordIndex (_other.currentRecordIndex),
      visitedRecords (_other.visitedRecords)
{
    assert (index);
    ++index->activeCursors;
}

VolumetricIndex::RayIntersectionCursorBase::RayIntersectionCursorBase (
    VolumetricIndex::RayIntersectionCursorBase &&_other) noexcept
    : index (_other.index),
      currentPoint (_other.currentPoint),
      direction (_other.direction),
      distanceTraveled (_other.distanceTraveled),
      ray (_other.ray),
      maxDistance (_other.maxDistance),
      currentCoordinate (_other.currentCoordinate),
      currentRecordIndex (_other.currentRecordIndex),
      visitedRecords (std::move (_other.visitedRecords))
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::RayIntersectionCursorBase::~RayIntersectionCursorBase () noexcept
{
    if (index)
    {
        --index->activeCursors;
    }
}

VolumetricIndex::RayIntersectionCursorBase::RayIntersectionCursorBase (
    VolumetricIndex *_index, const VolumetricIndex::RayContainer &_ray, float _maxDistance) noexcept
    : index (_index),
      distanceTraveled (0.0f),
      ray (_ray),
      maxDistance (_maxDistance),
      currentRecordIndex (0u),
      visitedRecords (index->nextRecordId, false)
{
    assert (index);
    assert (maxDistance >= 0.0f);
    ++index->activeCursors;

    DoWithCorrectTypeOperations (
        index->dimensions[0u].minBorderField,
        [this] (const auto &_operations)
        {
            using Operations = std::decay_t <decltype (_operations)>;
            using ValueType = typename Operations::ValueType;

            // TODO: For simplicity, we copy data from dimensions into suitable data structure. Maybe optimize this out?
            AxisAlignedShape <ValueType> bordersShape;
            const auto *lookupRay = reinterpret_cast <const Ray <ValueType> *> (&ray);

            for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
            {
                const Dimension &dimension = index->dimensions[dimensionIndex];
                bordersShape.Min (dimensionIndex) = *reinterpret_cast <const ValueType *> (&dimension.globalMinBorder);
                bordersShape.Max (dimensionIndex) = *reinterpret_cast <const ValueType *> (&dimension.globalMaxBorder);
            }

            float distanceToBorders;
            if (index->CheckRayShapeIntersection (
                ray, *reinterpret_cast <AxisAlignedShapeContainer *> (&bordersShape),
                distanceToBorders, currentPoint, _operations))
            {
                distanceTraveled += distanceToBorders;
                for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
                {
                    const Dimension &dimension = index->dimensions[dimensionIndex];
                    const SupportedAxisValue leafSize = index->CalculateLeafSize (dimension, _operations);
                    currentCoordinate[dimensionIndex] = index->CalculateCoordinate (
                        currentPoint[dimensionIndex], dimension, leafSize, _operations);

                    currentPoint[dimensionIndex] =
                        (currentPoint[dimensionIndex] - _operations.ToFloat (dimension.globalMinBorder)) /
                        _operations.ToFloat (leafSize);

                    direction[dimensionIndex] =
                        static_cast <float> (lookupRay->Direction (dimensionIndex)) /
                        _operations.ToFloat (leafSize);
                }

#ifndef NDEBUG
                // Assert that at least for one axis direction has non zero value.
                std::size_t nonZeroDirections = 0u;

                for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
                {
                    if (fabs (direction[dimensionIndex]) > Constants::VolumetricIndex::EPSILON)
                    {
                        ++nonZeroDirections;
                    }
                }

                assert (nonZeroDirections > 0u);
#endif

                FixCurrentRecordIndex ();
            }
            else
            {
                // If ray does not intersect index bounds, initialize cursor as finished.
                const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (index->dimensions.GetCount ());
                for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
                {
                    currentCoordinate[dimensionIndex] = maxCoordinate;
                }
            }
        }
    );
}

bool VolumetricIndex::RayIntersectionCursorBase::IsFinished () const noexcept
{
    assert (index);
    if (distanceTraveled > maxDistance)
    {
        return true;
    }

    const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (index->dimensions.GetCount ());
    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        if (currentCoordinate[dimensionIndex] >= maxCoordinate)
        {
            return true;
        }
    }

    return false;
}

void VolumetricIndex::RayIntersectionCursorBase::MoveToNextRecord () noexcept
{
    CursorCommons <RayIntersectionCursorBase>::MoveToNextRecord (*this);
}

const void *VolumetricIndex::RayIntersectionCursorBase::GetRecord () const noexcept
{
    assert (index);
    // TODO: Check performance of this method. IsFinished check can be too long.
    if (!IsFinished ())
    {
        const LeafData &leaf = index->leaves[index->GetLeafIndex (currentCoordinate)];
        assert (currentRecordIndex < leaf.records.size ());
        return leaf.records[currentRecordIndex].record;
    }
    else
    {
        return nullptr;
    }
}

VolumetricIndex *VolumetricIndex::RayIntersectionCursorBase::GetIndex () const noexcept
{
    return index;
}

void VolumetricIndex::RayIntersectionCursorBase::FixCurrentRecordIndex () noexcept
{
    CursorCommons <RayIntersectionCursorBase>::FixCurrentRecordIndex (*this);
}

template <typename Operations>
bool VolumetricIndex::RayIntersectionCursorBase::MoveToNextCoordinate (const Operations &) noexcept
{
    std::size_t closestDimension = std::numeric_limits <std::size_t>::max ();
    float minT = std::numeric_limits <float>::max ();

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        if (fabs (direction[dimensionIndex]) > Constants::VolumetricIndex::EPSILON)
        {
            const float step = direction[dimensionIndex] > 0.0f ? 1.0f : 0.0f;
            const float target = static_cast <float> (currentCoordinate[dimensionIndex]) + step;
            const float t = (target - currentPoint[dimensionIndex]) / direction[dimensionIndex];

            if (t < minT)
            {
                minT = t;
                closestDimension = dimensionIndex;
            }
        }
    }

    assert (closestDimension < index->dimensions.GetCount ());
    currentRecordIndex = 0u;

    if (direction[closestDimension] > 0.0f)
    {
        ++currentCoordinate[closestDimension];
    }
    else
    {
        --currentCoordinate[closestDimension];
    }

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        currentPoint[dimensionIndex] += minT * direction[dimensionIndex];
    }

    const auto *lookupRay = reinterpret_cast <const Ray <typename Operations::ValueType> *> (&ray);
    float deltaDistance = 0.0f;

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        float delta = lookupRay->Direction (dimensionIndex) * minT;
        deltaDistance += delta * delta;
    }

    distanceTraveled += sqrt (deltaDistance);
    return !IsFinished ();
}

template <typename Operations>
bool VolumetricIndex::RayIntersectionCursorBase::CheckIntersection (
    const void *_record, const Operations &_operations) const noexcept
{
    assert (index);
    // TODO: For simplicity, we copy data from record into suitable data structure. Maybe optimize this out?
    AxisAlignedShape <typename Operations::ValueType> recordShape;

    for (std::size_t dimensionIndex = 0u; dimensionIndex < index->dimensions.GetCount (); ++dimensionIndex)
    {
        const Dimension &dimension = index->dimensions[dimensionIndex];
        recordShape.Min (dimensionIndex) =
            *reinterpret_cast <const typename Operations::ValueType *> (dimension.minBorderField.GetValue (_record));

        recordShape.Max (dimensionIndex) =
            *reinterpret_cast <const typename Operations::ValueType *> (dimension.maxBorderField.GetValue (_record));
    }

    float distanceToShape;
    std::array <float, Constants::VolumetricIndex::MAX_DIMENSIONS> intersectionPoint;

    bool intersects = index->CheckRayShapeIntersection (
        ray, *reinterpret_cast <AxisAlignedShapeContainer *> (&recordShape),
        distanceToShape, intersectionPoint, _operations);

    return intersects && distanceToShape <= maxDistance;
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    const VolumetricIndex::RayIntersectionReadCursor &_other) noexcept
    : RayIntersectionCursorBase (_other)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricIndex::RayIntersectionReadCursor &&_other) noexcept
    : RayIntersectionCursorBase (std::move (_other))
{
}

VolumetricIndex::RayIntersectionReadCursor::~RayIntersectionReadCursor () noexcept
{
    if (GetIndex ())
    {
        GetIndex ()->storage->UnregisterReader ();
    }
}

const void *VolumetricIndex::RayIntersectionReadCursor::operator * () const noexcept
{
    return GetRecord ();
}

VolumetricIndex::RayIntersectionReadCursor &VolumetricIndex::RayIntersectionReadCursor::operator ++ () noexcept
{
    MoveToNextRecord ();
    return *this;
}

VolumetricIndex::RayIntersectionReadCursor::RayIntersectionReadCursor (
    VolumetricIndex *_index, const RayContainer &_ray, float _maxDistance) noexcept
    : RayIntersectionCursorBase (_index, _ray, _maxDistance)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterReader ();
}

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricIndex::RayIntersectionEditCursor &&_other) noexcept
    : RayIntersectionCursorBase (std::move (_other))
{
}

VolumetricIndex::RayIntersectionEditCursor::~RayIntersectionEditCursor () noexcept
{
    if (GetIndex ())
    {
        if (!IsFinished ())
        {
            // Record can be stored in many coordinates, therefore
            // edition by cursor can not be optimized for owner index.
            GetIndex ()->storage->EndRecordEdition (GetRecord (), nullptr);
        }

        GetIndex ()->storage->UnregisterWriter ();
    }
}

void *VolumetricIndex::RayIntersectionEditCursor::operator * () noexcept
{
    return const_cast <void *> (GetRecord ());
}

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator ~ () noexcept
{
    assert (!IsFinished ());
    // Record can be stored in many coordinates, therefore deletion by cursor can not be optimized for owner index.
    GetIndex ()->storage->DeleteRecord (**this, nullptr);

    FixCurrentRecordIndex ();
    BeginRecordEdition ();
    return *this;
}

VolumetricIndex::RayIntersectionEditCursor &VolumetricIndex::RayIntersectionEditCursor::operator ++ () noexcept
{
    assert (!IsFinished ());
    // Record can be stored in many coordinates, therefore edition by cursor can not be optimized for owner index.
    GetIndex ()->storage->EndRecordEdition (GetRecord (), nullptr);
    FixCurrentRecordIndex ();
    BeginRecordEdition ();
    return *this;
}

VolumetricIndex::RayIntersectionEditCursor::RayIntersectionEditCursor (
    VolumetricIndex *_index, const RayContainer &_ray, float _maxDistance) noexcept
    : RayIntersectionCursorBase (_index, _ray, _maxDistance)
{
    assert (GetIndex ());
    GetIndex ()->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void VolumetricIndex::RayIntersectionEditCursor::BeginRecordEdition () const noexcept
{
    assert (GetIndex ());
    if (!IsFinished ())
    {
        GetIndex ()->storage->BeginRecordEdition (GetRecord ());
    }
}

const InplaceVector <VolumetricIndex::Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> &
VolumetricIndex::GetDimensions () const noexcept
{
    return dimensions;
}

VolumetricIndex::ShapeIntersectionReadCursor VolumetricIndex::LookupShapeIntersectionToRead (
    const VolumetricIndex::AxisAlignedShapeContainer &_shape) noexcept
{
    return DoWithCorrectTypeOperations (
        dimensions[0u].minBorderField,
        [this, &_shape] (const auto &_operations)
        {
            return ShapeIntersectionReadCursor (this, CalculateSector (_shape, _operations), _shape);
        });
}

VolumetricIndex::ShapeIntersectionEditCursor VolumetricIndex::LookupShapeIntersectionToEdit (
    const VolumetricIndex::AxisAlignedShapeContainer &_shape) noexcept
{
    return DoWithCorrectTypeOperations (
        dimensions[0u].minBorderField,
        [this, &_shape] (const auto &_operations)
        {
            return ShapeIntersectionEditCursor (this, CalculateSector (_shape, _operations), _shape);
        });
}

VolumetricIndex::RayIntersectionReadCursor VolumetricIndex::LookupRayIntersectionToRead (
    const VolumetricIndex::RayContainer &_ray, float _maxDistance) noexcept
{
    return RayIntersectionReadCursor (this, _ray, _maxDistance);
}

VolumetricIndex::RayIntersectionEditCursor VolumetricIndex::LookupRayIntersectionToEdit (
    const VolumetricIndex::RayContainer &_ray, float _maxDistance) noexcept
{
    return RayIntersectionEditCursor (this, _ray, _maxDistance);
}

void VolumetricIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

std::vector <VolumetricIndex::RecordData>::iterator VolumetricIndex::LeafData::FindRecord (const void *_record) noexcept
{
    return std::find_if (
        records.begin (), records.end (),
        [_record] (const RecordData &_data)
        {
            return _record == _data.record;
        });
}

void VolumetricIndex::LeafData::DeleteRecord (
    const std::vector <VolumetricIndex::RecordData>::iterator &_recordIterator) noexcept
{
    assert (_recordIterator != records.end ());
    if (_recordIterator + 1 != records.end ())
    {
        *_recordIterator = records.back ();
    }

    records.pop_back ();
}

VolumetricIndex::VolumetricIndex (Storage *_storage, const std::vector <DimensionDescriptor> &_dimensions) noexcept
    : IndexBase (_storage),
      dimensions (),
      leaves (),
      freeRecordIds (),
      nextRecordId (0u)
{
    assert (!_dimensions.empty ());
    assert (_dimensions.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    std::size_t dimensionCount = std::min (_dimensions.size (), Constants::VolumetricIndex::MAX_DIMENSIONS);
    leaves.resize (std::size_t (1u) << (
        dimensionCount * (Constants::VolumetricIndex::LEVELS[dimensionCount - 1u] - 1u)));

#ifndef NDEBUG
    // Current implementation expects that all fields have same archetype and size.
    StandardLayout::Field firstDimensionMinField = _storage->GetRecordMapping ().GetField (
        _dimensions[0u].minBorderField);

    assert (firstDimensionMinField.IsHandleValid ());
    StandardLayout::FieldArchetype expectedArchetype = firstDimensionMinField.GetArchetype ();
    std::size_t expectedSize = firstDimensionMinField.GetSize ();
#endif

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensionCount; ++dimensionIndex)
    {
        const DimensionDescriptor &descriptor = _dimensions[dimensionIndex];
        StandardLayout::Field minField = _storage->GetRecordMapping ().GetField (descriptor.minBorderField);
        StandardLayout::Field maxField = _storage->GetRecordMapping ().GetField (descriptor.maxBorderField);

        assert (minField.GetArchetype () == expectedArchetype);
        assert (minField.GetSize () == expectedSize);

        assert (maxField.GetArchetype () == expectedArchetype);
        assert (maxField.GetSize () == expectedSize);

        dimensions.EmplaceBack (Dimension {
            minField, descriptor.globalMinBorder,
            maxField, descriptor.globalMaxBorder});
    }
}

template <typename Operations>
VolumetricIndex::LeafSector VolumetricIndex::CalculateSector (
    const void *_record, const Operations &_operations) const noexcept
{
    LeafSector sector {};
    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        const Dimension &dimension = dimensions[dimensionIndex];
        const SupportedAxisValue min = *reinterpret_cast <const SupportedAxisValue *> (
            dimension.minBorderField.GetValue (_record));

        const SupportedAxisValue max = *reinterpret_cast <const SupportedAxisValue *> (
            dimension.maxBorderField.GetValue (_record));

        assert (_operations.Compare (min, max) <= 0);
        const SupportedAxisValue leafSize = CalculateLeafSize (dimension, _operations);
        sector.min[dimensionIndex] = CalculateCoordinate (min, dimension, leafSize, _operations);
        sector.max[dimensionIndex] = CalculateCoordinate (max, dimension, leafSize, _operations);
    }

    return sector;
}

template <typename Operations>
VolumetricIndex::LeafSector VolumetricIndex::CalculateSector (
    const VolumetricIndex::AxisAlignedShapeContainer &_shape, const Operations &_operations) const noexcept
{
    const auto *lookupShape = reinterpret_cast <const AxisAlignedShape <typename Operations::ValueType> *> (&_shape);
    LeafSector sector {};

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        const Dimension &dimension = dimensions[dimensionIndex];
        const auto &min = lookupShape->Min (dimensionIndex);
        const auto &max = lookupShape->Max (dimensionIndex);

        assert (min <= max);
        const SupportedAxisValue leafSize = CalculateLeafSize (dimension, _operations);
        sector.min[dimensionIndex] = CalculateCoordinate (min, dimension, leafSize, _operations);
        sector.max[dimensionIndex] = CalculateCoordinate (max, dimension, leafSize, _operations);
    }

    return sector;
}

template <typename Operations>
VolumetricIndex::SupportedAxisValue VolumetricIndex::CalculateLeafSize (
    const VolumetricIndex::Dimension &_dimension, const Operations &_operations) const noexcept
{
    return _operations.Divide (
        _operations.Subtract (_dimension.globalMaxBorder, _dimension.globalMinBorder),
        GetMaxLeafCoordinateOnAxis (dimensions.GetCount ()));
}

template <typename Operations>
std::size_t VolumetricIndex::CalculateCoordinate (
    const VolumetricIndex::SupportedAxisValue &_value, const VolumetricIndex::Dimension &_dimension,
    const VolumetricIndex::SupportedAxisValue &_leafSize, const Operations &_operations) const noexcept
{
    const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (dimensions.GetCount ());
    return std::clamp <std::size_t> (
        _operations.TruncateToSizeType (_operations.Divide (
            _operations.Subtract (_value, _dimension.globalMinBorder), _leafSize)),
        0u, maxCoordinate - 1u);
}

template <typename Operations>
bool VolumetricIndex::CheckRayShapeIntersection (
    const VolumetricIndex::RayContainer &_ray, const VolumetricIndex::AxisAlignedShapeContainer &_shape,
    float &_distanceOutput, std::array <float, Constants::VolumetricIndex::MAX_DIMENSIONS> &_intersectionPointOutput,
    const Operations &_operations) const noexcept
{
    const auto *lookupRay = reinterpret_cast <const Ray <typename Operations::ValueType> *> (&_ray);
    const auto *lookupShape = reinterpret_cast <const AxisAlignedShape <typename Operations::ValueType> *> (&_shape);
    bool inside = true;

    // point = ray.origin + ray.direction * T
    float maxT = 0.0f;
    std::size_t maxTDimension = std::numeric_limits <std::size_t>::max ();

    const auto calculateT =
        [&_operations, lookupRay] (std::size_t _dimensionIndex, const SupportedAxisValue &_cornerValue)
        {
            const float direction = static_cast <float> (lookupRay->Direction (_dimensionIndex));
            if (fabs (direction) > Constants::VolumetricIndex::EPSILON)
            {
                float distance = _operations.ToFloat (
                    _operations.Subtract (_cornerValue, lookupRay->Origin (_dimensionIndex)));
                return distance / direction;
            }
            else
            {
                return 0.0f;
            }
        };

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        float t = 0.0f;
        if (_operations.Compare (lookupRay->Origin (dimensionIndex), lookupShape->Min (dimensionIndex)) < 0)
        {
            inside = false;
            t = calculateT (dimensionIndex, lookupShape->Min (dimensionIndex));
        }
        else if (_operations.Compare (lookupRay->Origin (dimensionIndex), lookupShape->Max (dimensionIndex)) > 0)
        {
            inside = false;
            t = calculateT (dimensionIndex, lookupShape->Max (dimensionIndex));
        }

        if (t > maxT)
        {
            maxT = t;
            maxTDimension = dimensionIndex;
        }
    }

    if (inside)
    {
        for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
        {
            _intersectionPointOutput[dimensionIndex] = static_cast <float> (lookupRay->Origin (dimensionIndex));
        }

        _distanceOutput = 0.0f;
        return true;
    }

    // If expression below is true, ray origin is inside on all dimensions except one, in which
    // ray is parallel to shape. Therefore there is no intersection between ray and shape.
    if (maxTDimension >= dimensions.GetCount ())
    {
        return false;
    }

    // Ignore backward intersections.
    if (maxT < 0.0f)
    {
        return false;
    }

    _distanceOutput = 0.0f;
    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        const float origin = static_cast <float> (lookupRay->Origin (dimensionIndex));
        const float direction = static_cast <float> (lookupRay->Direction (dimensionIndex));
        const float step = direction * maxT;

        _intersectionPointOutput[dimensionIndex] = origin + step;
        _distanceOutput += step * step;

        if (dimensionIndex != maxTDimension)
        {
            if (_intersectionPointOutput[dimensionIndex] < static_cast <float> (lookupShape->Min (dimensionIndex)) ||
                _intersectionPointOutput[dimensionIndex] > static_cast <float> (lookupShape->Max (dimensionIndex)))
            {
                return false;
            }
        }
    }

    _distanceOutput = sqrt (_distanceOutput);
    return true;
}

template <typename Callback>
void VolumetricIndex::ForEachCoordinate (
    const VolumetricIndex::LeafSector &_sector, const Callback &_callback) const noexcept
{
    LeafCoordinate coordinate = _sector.min;
    while (true)
    {
        _callback (coordinate);
        if (AreEqual (coordinate, _sector.max))
        {
            break;
        }
        else
        {
            coordinate = NextInsideSector (_sector, coordinate);
        }
    }
}

std::size_t VolumetricIndex::GetLeafIndex (const VolumetricIndex::LeafCoordinate &_coordinate) const noexcept
{
    std::size_t result = 0u;
    const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (dimensions.GetCount ());

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        assert (_coordinate[dimensionIndex] < maxCoordinate);
        result = result * maxCoordinate + _coordinate[dimensionIndex];
    }

    return result;
}

bool VolumetricIndex::IsInsideSector (
    const VolumetricIndex::LeafSector &_sector, const VolumetricIndex::LeafCoordinate &_coordinate) const noexcept
{
    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        if (_coordinate[dimensionIndex] < _sector.min[dimensionIndex] ||
            _coordinate[dimensionIndex] > _sector.max[dimensionIndex])
        {
            return false;
        }
    }

    return true;
}

bool VolumetricIndex::AreEqual (
    const VolumetricIndex::LeafCoordinate &_left, const VolumetricIndex::LeafCoordinate &_right) const noexcept
{
    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        if (_left[dimensionIndex] != _right[dimensionIndex])
        {
            return false;
        }
    }

    return true;
}

VolumetricIndex::LeafCoordinate VolumetricIndex::NextInsideSector (
    const VolumetricIndex::LeafSector &_sector, VolumetricIndex::LeafCoordinate _coordinate) const noexcept
{
    assert (IsInsideSector (_sector, _coordinate));
    assert (!AreEqual (_sector.max, _coordinate));

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        if (_coordinate[dimensionIndex] < _sector.max[dimensionIndex])
        {
            ++_coordinate[dimensionIndex];
            break;
        }
        else
        {
            _coordinate[dimensionIndex] = _sector.min[dimensionIndex];
        }
    }

    return _coordinate;
}

bool VolumetricIndex::AreEqual (
    const VolumetricIndex::LeafSector &_left, const VolumetricIndex::LeafSector &_right) const noexcept
{
    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        if (_left.min[dimensionIndex] != _right.min[dimensionIndex] ||
            _left.max[dimensionIndex] != _right.max[dimensionIndex])
        {
            return false;
        }
    }

    return true;
}

void VolumetricIndex::InsertRecord (const void *_record) noexcept
{
    assert (_record);
    std::size_t recordId;

    if (!freeRecordIds.empty ())
    {
        recordId = freeRecordIds.back ();
        freeRecordIds.pop_back ();
    }
    else
    {
        recordId = nextRecordId++;
    }

    LeafSector sector = DoWithCorrectTypeOperations (
        dimensions[0u].minBorderField,
        [this, _record] (const auto &_operations)
        {
            return CalculateSector (_record, _operations);
        });

    ForEachCoordinate (
        sector,
        [this, _record, recordId] (const LeafCoordinate &_coordinate)
        {
            std::size_t index = GetLeafIndex (_coordinate);
            leaves[index].records.emplace_back (RecordData {_record, recordId});
        });
}

void VolumetricIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    assert (_record);
    LeafSector sector = DoWithCorrectTypeOperations (
        dimensions[0u].minBorderField,
        [this, _recordBackup] (const auto &_operations)
        {
            return CalculateSector (_recordBackup, _operations);
        });

    std::size_t recordId;
    ForEachCoordinate (
        sector,
        [this, _record, &recordId] (const LeafCoordinate &_coordinate)
        {
            LeafData &leaf = leaves[GetLeafIndex (_coordinate)];
            auto recordIterator = leaf.FindRecord (_record);
            assert (recordIterator != leaf.records.end ());

            recordId = recordIterator->recordId;
            leaf.DeleteRecord (recordIterator);
        });

    freeRecordIds.emplace_back (recordId);
}

void VolumetricIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    DoWithCorrectTypeOperations (
        dimensions[0u].minBorderField,
        [this, _record, _recordBackup] (const auto &_operations)
        {
            const LeafSector oldSector = CalculateSector (_recordBackup, _operations);
            const LeafSector newSector = CalculateSector (_record, _operations);

            if (!AreEqual (oldSector, newSector))
            {
                // TODO: For now we use simple logic, because complex optimizations for large objects
                //       could harm performance for small objects, because these optimizations require
                //       additional sector checks. Revisit it later.

                // Write invalid initial value into recordId, so we could check if recordId is already
                // found and skip unnecessary record search in coordinates that are not excluded.
                std::size_t recordId = nextRecordId;

                ForEachCoordinate (
                    oldSector,
                    [this, _record, &recordId, &newSector] (const LeafCoordinate &_coordinate)
                    {
                        bool excluded = IsInsideSector (newSector, _coordinate);
                        if (excluded || recordId == nextRecordId)
                        {
                            LeafData &leaf = leaves[GetLeafIndex (_coordinate)];
                            auto recordIterator = leaf.FindRecord (_record);
                            assert (recordIterator != leaf.records.end ());

                            recordId = recordIterator->recordId;
                            if (excluded)
                            {
                                leaf.DeleteRecord (recordIterator);
                            }
                        }
                    });

                ForEachCoordinate (
                    newSector,
                    [this, _record, &recordId, &oldSector] (const LeafCoordinate &_coordinate)
                    {
                        if (!IsInsideSector (oldSector, _coordinate))
                        {
                            std::size_t index = GetLeafIndex (_coordinate);
                            leaves[index].records.emplace_back (RecordData {_record, recordId});
                        }
                    });
            }
        });
}

void VolumetricIndex::OnWriterClosed () noexcept
{
    // All changes and deletions should be processed on the spot.
}

template <typename Type>
int TypeOperations <Type>::Compare (
    const VolumetricIndex::SupportedAxisValue &_left, const VolumetricIndex::SupportedAxisValue &_right) const noexcept
{
    return comparator.Compare (&_left, &_right);
}

template <typename Type>
int TypeOperations <Type>::Compare (const void *_left, const void *_right) const noexcept
{
    return comparator.Compare (_left, _right);
}

template <typename Type>
VolumetricIndex::SupportedAxisValue TypeOperations <Type>::Subtract (
    const VolumetricIndex::SupportedAxisValue &_left, const VolumetricIndex::SupportedAxisValue &_right) const noexcept
{
    return *reinterpret_cast <const Type *> (&_left) - *reinterpret_cast <const Type *> (&_right);
}

template <typename Type>
VolumetricIndex::SupportedAxisValue TypeOperations <Type>::Divide (
    const VolumetricIndex::SupportedAxisValue &_value,
    const VolumetricIndex::SupportedAxisValue &_divider) const noexcept
{
    return *reinterpret_cast <const Type *> (&_value) / *reinterpret_cast <const Type *> (&_divider);
}

template <typename Type>
VolumetricIndex::SupportedAxisValue
TypeOperations <Type>::Divide (const VolumetricIndex::SupportedAxisValue &_value, std::size_t divider) const noexcept
{
    assert (divider);
    return *reinterpret_cast <const Type *> (&_value) / static_cast <Type> (divider);
}

template <typename Type>
std::size_t TypeOperations <Type>::TruncateToSizeType (
    const VolumetricIndex::SupportedAxisValue &_value) const noexcept
{
    return static_cast <std::size_t> (*reinterpret_cast <const Type *> (&_value));
}

template <typename Type>
float TypeOperations <Type>::ToFloat (const VolumetricIndex::SupportedAxisValue &_value) const noexcept
{
    return static_cast <float> (*reinterpret_cast <const Type *> (&_value));
}

template <typename Cursor>
void CursorCommons <Cursor>::MoveToNextRecord (Cursor &_cursor) noexcept
{
    DoWithCorrectTypeOperations (
        _cursor.index->dimensions[0u].minBorderField,
        [&_cursor] (const auto &_operations)
        {
            assert (_cursor.index);
            assert (!_cursor.IsFinished ());

            ++_cursor.currentRecordIndex;
            const VolumetricIndex::LeafData *leaf = &_cursor.index->leaves[
                _cursor.index->GetLeafIndex (_cursor.currentCoordinate)];
            bool overflow;

            while ((overflow = _cursor.currentRecordIndex >= leaf->records.size ()) ||
                   _cursor.visitedRecords[leaf->records[_cursor.currentRecordIndex].recordId] ||
                   !_cursor.CheckIntersection (leaf->records[_cursor.currentRecordIndex].record, _operations))
            {
                if (overflow)
                {
                    if (_cursor.MoveToNextCoordinate (_operations))
                    {
                        leaf = &_cursor.index->leaves[_cursor.index->GetLeafIndex (_cursor.currentCoordinate)];
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    // Mark record as visited to avoid unnecessary checks in other leaves.
                    _cursor.visitedRecords[leaf->records[_cursor.currentRecordIndex].recordId] = true;
                    ++_cursor.currentRecordIndex;
                }
            }

            if (!overflow)
            {
                _cursor.visitedRecords[leaf->records[_cursor.currentRecordIndex].recordId] = true;
            }
        });
}

template <typename Cursor>
void CursorCommons <Cursor>::FixCurrentRecordIndex (Cursor &_cursor) noexcept
{
    assert (_cursor.index);
    DoWithCorrectTypeOperations (
        _cursor.index->dimensions[0u].minBorderField,
        [&_cursor] (const auto &_operations)
        {
            if (!_cursor.IsFinished ())
            {
                const VolumetricIndex::LeafData &leaf = _cursor.index->leaves[
                    _cursor.index->GetLeafIndex (_cursor.currentCoordinate)];

                if (_cursor.currentRecordIndex >= leaf.records.size () ||
                    _cursor.visitedRecords[leaf.records[_cursor.currentRecordIndex].recordId] ||
                    !_cursor.CheckIntersection (leaf.records[_cursor.currentRecordIndex].record, _operations))
                {
                    _cursor.MoveToNextRecord ();
                }
                else
                {
                    // Ensure that current record is visited. Visitation marks are added during cursor movement,
                    // therefore after cursor construction or record deletion current record could be unvisited.
                    _cursor.visitedRecords[leaf.records[_cursor.currentRecordIndex].recordId] = true;
                }
            }
        });
}
} // namespace Emergence::Pegasus