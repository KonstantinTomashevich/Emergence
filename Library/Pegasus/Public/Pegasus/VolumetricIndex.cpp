#include <algorithm>
#include <cassert>

#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>
#include <Pegasus/VolumetricIndex.hpp>

namespace Emergence::Pegasus
{
constexpr std::size_t GetMaxLeafCoordinateOnAxis (std::size_t _subdivisions)
{
    return 1u << Constants::VolumetricIndex::LEVELS[_subdivisions - 1u];
}

template <typename Type>
struct TypeOperations final
{
    int Compare (const VolumetricIndex::SupportedAxisValue &_left,
                 const VolumetricIndex::SupportedAxisValue &_right) const noexcept;

    VolumetricIndex::SupportedAxisValue Subtract (
        const VolumetricIndex::SupportedAxisValue &_left,
        const VolumetricIndex::SupportedAxisValue &_right) const noexcept;

    VolumetricIndex::SupportedAxisValue Divide (
        const VolumetricIndex::SupportedAxisValue &_value,
        const VolumetricIndex::SupportedAxisValue &divider) const noexcept;

    VolumetricIndex::SupportedAxisValue Divide (
        const VolumetricIndex::SupportedAxisValue &_value, std::size_t divider) const noexcept;

    std::size_t TruncateToSizeType (const VolumetricIndex::SupportedAxisValue &_value) const noexcept;

private:
    NumericValueComparator <Type> comparator {};
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

VolumetricIndex::CursorBase::CursorBase (const VolumetricIndex::CursorBase &_other) noexcept
    : index (_other.index),
      sector (_other.sector),
      currentCoordinate (_other.currentCoordinate),
      currentRecordIndex (_other.currentRecordIndex),
      visitedRecords (_other.visitedRecords)
{
    assert (index);
}

VolumetricIndex::CursorBase::CursorBase (VolumetricIndex::CursorBase &&_other) noexcept
    : index (_other.index),
      sector (std::move (_other.sector)),
      currentCoordinate (std::move (_other.currentCoordinate)),
      currentRecordIndex (std::move (_other.currentRecordIndex)),
      visitedRecords (std::move (_other.visitedRecords))
{
    assert (index);
    _other.index = nullptr;
}

VolumetricIndex::CursorBase::~CursorBase () noexcept
{
    if (index)
    {
        --index->activeCursors;
    }
}

VolumetricIndex::CursorBase::CursorBase (
    VolumetricIndex *_index, VolumetricIndex::LeafSector _sector,
    VolumetricIndex::LeafCoordinate _currentCoordinate, size_t _currentRecordIndex) noexcept
    : index (_index),
      sector (std::move (_sector)),
      currentCoordinate (std::move (_currentCoordinate)),
      currentRecordIndex (std::move (_currentRecordIndex)),
      visitedRecords (index->nextRecordId, false)
{
    assert (index);
    assert (index->IsInsideSector (sector, currentCoordinate));
}

bool VolumetricIndex::CursorBase::IsFinished () const noexcept
{
    assert (index);
    return index->AreEqual (currentCoordinate, sector.max) &&
           currentRecordIndex >= index->leaves[index->GetLeafIndex (currentCoordinate)].records.size ();
}

void VolumetricIndex::CursorBase::MoveToNextRecord () noexcept
{
    assert (index);
    assert (!IsFinished ());

    ++currentRecordIndex;
    const LeafData *leaf = &index->leaves[index->GetLeafIndex (currentCoordinate)];
    bool overflow;

    while ((overflow = currentRecordIndex >= leaf->records.size ()) ||
           visitedRecords[leaf->records[currentRecordIndex].recordId])
    {
        if (overflow)
        {
            if (currentCoordinate == sector.max)
            {
                break;
            }

            currentRecordIndex = 0u;
            currentCoordinate = index->NextInsideSector (sector, currentCoordinate);
            leaf = &index->leaves[index->GetLeafIndex (currentCoordinate)];
        }
        else
        {
            ++currentRecordIndex;
        }
    }

    if (!overflow)
    {
        visitedRecords[leaf->records[currentRecordIndex].recordId] = true;
    }
}

const void *VolumetricIndex::CursorBase::GetRecord () const noexcept
{
    assert (index);
    const LeafData &leaf = index->leaves[index->GetLeafIndex (currentCoordinate)];
    assert (currentRecordIndex < leaf.records.size () || IsFinished ());
    return currentRecordIndex < leaf.records.size () ? leaf.records[currentRecordIndex].record : nullptr;
}

void VolumetricIndex::CursorBase::FixCurrentRecordIndex () noexcept
{
    if (!IsFinished () && currentRecordIndex > index->leaves[index->GetLeafIndex (currentCoordinate)].records.size ())
    {
        MoveToNextRecord ();
    }
}

const InplaceVector <VolumetricIndex::Dimension, Constants::VolumetricIndex::MAX_DIMENSIONS> &
VolumetricIndex::GetDimensions () const noexcept
{
    return dimensions;
}

void VolumetricIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

template <typename Operations>
VolumetricIndex::LeafSector VolumetricIndex::CalculateSector (
    const void *_record, const Operations &_operations) const noexcept
{
    LeafSector sector;
    const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (dimensions.GetCount ());

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
        const Dimension &dimension = dimensions[dimensionIndex];
        const SupportedAxisValue min = *reinterpret_cast <const SupportedAxisValue *> (
            dimension.minBorderField.GetValue (_record));

        const SupportedAxisValue max = *reinterpret_cast <const SupportedAxisValue *> (
            dimension.minBorderField.GetValue (_record));

        assert (_operations.Compare (min, max) <= 0);
        const SupportedAxisValue leafSize = _operations.Divide (
            _operations.Subtract (dimension.globalMaxBorder, dimension.globalMinBorder), maxCoordinate);

        sector.min[dimensionIndex] = std::clamp <std::size_t> (
            0u, maxCoordinate - 1u, _operations.TruncateToSizeType (_operations.Divide (
                _operations.Subtract (min, dimension.globalMinBorder), leafSize)));

        sector.max[dimensionIndex] = std::clamp <std::size_t> (
            0u, maxCoordinate - 1u, _operations.TruncateToSizeType (_operations.Divide (
                _operations.Subtract (max, dimension.globalMinBorder), leafSize)));
    }

    return sector;
}

std::size_t VolumetricIndex::GetLeafIndex (const VolumetricIndex::LeafCoordinate &_coordinate) const noexcept
{
    std::size_t result = 0u;
    const std::size_t maxCoordinate = GetMaxLeafCoordinateOnAxis (dimensions.GetCount ());

    for (std::size_t dimensionIndex = 0u; dimensionIndex < dimensions.GetCount (); ++dimensionIndex)
    {
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

void VolumetricIndex::InsertRecord (const void *_record) noexcept
{
    // TODO: Is there a quick way to assert that record is not already inserted?
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

    LeafCoordinate coordinate = sector.min;
    while (true)
    {
        std::size_t index = GetLeafIndex (coordinate);
        leaves[index].records.emplace_back (RecordData {_record, recordId});

        if (coordinate == sector.max)
        {
            break;
        }
        else
        {
            coordinate = NextInsideSector (sector, coordinate);
        }
    }
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

    LeafCoordinate coordinate = sector.min;
    std::size_t recordId;

    while (true)
    {
        std::size_t index = GetLeafIndex (coordinate);
        std::vector <RecordData> &records = leaves[index].records;

        auto iterator = std::find_if (
            records.begin (), records.end (),
            [_record] (const RecordData &_data)
            {
                return _record == _data.record;
            });

        assert (iterator != records.end ());
        recordId = iterator->recordId;

        if (iterator + 1 != records.end ())
        {
            *iterator = records.back ();
        }

        records.pop_back ();
        if (coordinate == sector.max)
        {
            break;
        }
        else
        {
            coordinate = NextInsideSector (sector, coordinate);
        }
    }

    freeRecordIds.emplace_back (recordId);
}

void VolumetricIndex::DeleteRecordMyself (VolumetricIndex::CursorBase &_cursor) noexcept
{
    const void *record = _cursor.GetRecord ();
    assert (record);
    storage->DeleteRecord (const_cast <void *> (record), this);
    _cursor.FixCurrentRecordIndex ();
}

template <typename Type>
int TypeOperations <Type>::Compare (
    const VolumetricIndex::SupportedAxisValue &_left, const VolumetricIndex::SupportedAxisValue &_right) const noexcept
{
    return comparator.Compare (&_left, &_right);
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

} // namespace Emergence::Pegasus