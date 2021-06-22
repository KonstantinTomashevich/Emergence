#include <algorithm>
#include <cassert>

#include <Pegasus/Constants/OrderedIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
OrderedIndex::ReadCursor::ReadCursor (const OrderedIndex::ReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReadCursor::ReadCursor (OrderedIndex::ReadCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReadCursor::~ReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *OrderedIndex::ReadCursor::operator * () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

OrderedIndex::ReadCursor &OrderedIndex::ReadCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

OrderedIndex::ReadCursor::ReadCursor (
    OrderedIndex *_index, std::vector <const void *>::const_iterator _begin,
    std::vector <const void *>::const_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::EditCursor::EditCursor (OrderedIndex::EditCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::EditCursor::~EditCursor () noexcept
{
    if (index)
    {
        if (current != end && index->storage->EndRecordEdition (*current, index))
        {
            index->OnRecordChangedByMe (current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *OrderedIndex::EditCursor::operator * () noexcept
{
    assert (index);
    return current != end ? const_cast <void *> (*current) : nullptr;
}

OrderedIndex::EditCursor &OrderedIndex::EditCursor::operator ~ ()
{
    assert (index);
    assert (current != end);

    index->DeleteRecordMyself (current);
    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::EditCursor &OrderedIndex::EditCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    if (index->storage->EndRecordEdition (*current, index))
    {
        index->OnRecordChangedByMe (current);
    }

    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::EditCursor::EditCursor (
    OrderedIndex *_index, std::vector <const void *>::iterator _begin,
    std::vector <const void *>::iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void OrderedIndex::EditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (const OrderedIndex::ReversedReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (OrderedIndex::ReversedReadCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReversedReadCursor::~ReversedReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *OrderedIndex::ReversedReadCursor::operator * () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

OrderedIndex::ReversedReadCursor &OrderedIndex::ReversedReadCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (
    OrderedIndex *_index, std::vector <const void *>::const_reverse_iterator _begin,
    std::vector <const void *>::const_reverse_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReversedEditCursor::ReversedEditCursor (OrderedIndex::ReversedEditCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReversedEditCursor::~ReversedEditCursor () noexcept
{
    if (index)
    {
        if (current != end && index->storage->EndRecordEdition (*current, index))
        {
            index->OnRecordChangedByMe (current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *OrderedIndex::ReversedEditCursor::operator * () noexcept
{
    assert (index);
    return current != end ? const_cast <void *> (*current) : nullptr;
}

OrderedIndex::ReversedEditCursor &OrderedIndex::ReversedEditCursor::operator ~ ()
{
    assert (index);
    assert (current != end);

    index->DeleteRecordMyself (current);
    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::ReversedEditCursor &OrderedIndex::ReversedEditCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    if (index->storage->EndRecordEdition (*current, index))
    {
        index->OnRecordChangedByMe (current);
    }

    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::ReversedEditCursor::ReversedEditCursor (
    OrderedIndex *_index, std::vector <const void *>::reverse_iterator _begin,
    std::vector <const void *>::reverse_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void OrderedIndex::ReversedEditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

OrderedIndex::ReadCursor OrderedIndex::LookupToRead (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReadCursor (this, result.begin, result.end);
}

OrderedIndex::ReversedReadCursor OrderedIndex::LookupToReadReversed (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReversedReadCursor (
        this, std::vector <const void *>::const_reverse_iterator (result.end),
        std::vector <const void *>::const_reverse_iterator (result.begin));
}

OrderedIndex::EditCursor OrderedIndex::LookupToEdit (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::EditCursor (this, result.begin, result.end);
}

OrderedIndex::ReversedEditCursor OrderedIndex::LookupToEditReversed (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReversedEditCursor (
        this, std::vector <const void *>::reverse_iterator (result.end),
        std::vector <const void *>::reverse_iterator (result.begin));
}

StandardLayout::Field OrderedIndex::GetIndexedField () const noexcept
{
    return indexedField;
}

void OrderedIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

bool OrderedIndex::Comparator::operator () (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    assert (_firstRecord);
    assert (_secondRecord);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (field.GetValue (_firstRecord), field.GetValue (_secondRecord), field);
}

bool OrderedIndex::Comparator::operator () (const OrderedIndex::Bound &_bound, const void *_record) const noexcept
{
    assert (_bound.boundValue);
    assert (_record);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (_bound.boundValue, field.GetValue (_record), field);
}

bool OrderedIndex::Comparator::operator () (const void *_record, const OrderedIndex::Bound &_bound) const noexcept
{
    assert (_record);
    assert (_bound.boundValue);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (field.GetValue (_record), _bound.boundValue, field);
}

OrderedIndex::MassInsertionExecutor::~MassInsertionExecutor () noexcept
{
    assert (owner);
    std::sort (owner->records.begin (), owner->records.end (), Comparator {owner});
}

void OrderedIndex::MassInsertionExecutor::InsertRecord (const void *_record) noexcept
{
    assert (owner);
    assert (_record);
    owner->records.emplace_back (_record);
}

OrderedIndex::MassInsertionExecutor::MassInsertionExecutor (OrderedIndex *_owner) noexcept
    : owner (_owner)
{
    assert (owner);
}

OrderedIndex::OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField)
    : IndexBase (_owner),
      indexedField (_owner->GetRecordMapping ().GetField (_indexedField))
{
    assert (indexedField.IsHandleValid ());
}

OrderedIndex::InternalLookupResult OrderedIndex::InternalLookup (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    assert (!_min.boundValue || !_max.boundValue ||
            !IsFieldValueLesser (_max.boundValue, _min.boundValue, indexedField));
    InternalLookupResult result {records.begin (), records.end ()};

    if (_min.boundValue)
    {
        result.begin = std::lower_bound (records.begin (), records.end (),
                                         _min, Comparator {this});
    }

    if (_max.boundValue)
    {
        result.end = std::upper_bound (result.begin, records.end (),
                                       _max, Comparator {this});
    }

    return result;
}

std::vector <const void *>::const_iterator OrderedIndex::LocateRecord (
    const void *_record, const void *_recordBackup) const noexcept
{
    auto iterator = std::lower_bound (records.begin (), records.end (),
                                      _recordBackup, Comparator {this});
    assert (iterator != records.end ());

    while (*iterator != _record)
    {
        ++iterator;
        assert (iterator != records.end ());
    }

    return iterator;
}

void OrderedIndex::InsertRecord (const void *_record) noexcept
{
    // TODO: Assert that record is not inserted already? Is there any way to check this fast?
    assert (_record);
    auto place = std::upper_bound (records.begin (), records.end (),
                                   _record, Comparator {this});
    records.insert (place, _record);
}

OrderedIndex::MassInsertionExecutor OrderedIndex::StartMassInsertion () noexcept
{
    return MassInsertionExecutor (this);
}

void OrderedIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    auto iterator = LocateRecord (_record, _recordBackup);
    std::size_t recordIndex = iterator - records.begin ();

    auto insertionPoint = std::lower_bound (
        deletedRecordIndices.begin (), deletedRecordIndices.end (), recordIndex);

    assert (insertionPoint == deletedRecordIndices.end () || *insertionPoint != recordIndex);
    deletedRecordIndices.emplace (insertionPoint, recordIndex);
}

void OrderedIndex::DeleteRecordMyself (const std::vector <const void *>::iterator &_position) noexcept
{
    assert (_position != records.end ());
    std::size_t index = _position - records.begin ();

    assert (deletedRecordIndices.empty () || index > deletedRecordIndices.back ());
    deletedRecordIndices.emplace_back (index);
    storage->DeleteRecord (const_cast <void *> (*_position), this);
}

void OrderedIndex::DeleteRecordMyself (const std::vector <const void *>::reverse_iterator &_position) noexcept
{
    assert (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;

    assert (deletedRecordIndices.empty () || index < deletedRecordIndices.front ());
    deletedRecordIndices.emplace (deletedRecordIndices.begin (), index);
    storage->DeleteRecord (const_cast <void *> (*_position), this);
}

void OrderedIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    auto iterator = LocateRecord (_record, _recordBackup);
    std::size_t recordIndex = iterator - records.begin ();

    auto insertionPoint = std::lower_bound (
        changedRecords.begin (), changedRecords.end (), recordIndex,
        [] (const ChangedRecordInfo &_changedRecordInfo, size_t _indexToInsert) -> bool
        {
            return _changedRecordInfo.originalIndex < _indexToInsert;
        });

    assert (insertionPoint == changedRecords.end () || insertionPoint->originalIndex != recordIndex);
    changedRecords.emplace (insertionPoint, ChangedRecordInfo {recordIndex, (_record)});
}

void OrderedIndex::OnRecordChangedByMe (const std::vector <const void *>::iterator &_position) noexcept
{
    assert (_position != records.end ());
    std::size_t index = _position - records.begin ();

    assert (changedRecords.empty () || index > changedRecords.back ().originalIndex);
    changedRecords.emplace_back (ChangedRecordInfo {index, *_position});
}

void OrderedIndex::OnRecordChangedByMe (const std::vector <const void *>::reverse_iterator &_position) noexcept
{
    assert (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;
    assert (changedRecords.empty () || index < changedRecords.front ().originalIndex);
    changedRecords.emplace (changedRecords.begin (), ChangedRecordInfo {index, *_position});
}

void OrderedIndex::OnWriterClosed () noexcept
{
    if (!changedRecords.empty () || !deletedRecordIndices.empty ())
    {
        auto changedRecordsIterator = changedRecords.begin ();
        const auto changedRecordsEnd = changedRecords.end ();

        auto deletedRecordsIterator = deletedRecordIndices.begin ();
        const auto deletedRecordsEnd = deletedRecordIndices.end ();

        auto AdvanceToNextCheckpoint =
            [&changedRecordsIterator, &changedRecordsEnd, &deletedRecordsIterator, &deletedRecordsEnd] () -> std::size_t
            {
                std::size_t nextCheckpoint;
                if (changedRecordsIterator != changedRecordsEnd)
                {
                    if (deletedRecordsIterator != deletedRecordsEnd &&
                        *deletedRecordsIterator < changedRecordsIterator->originalIndex)
                    {
                        nextCheckpoint = *deletedRecordsIterator;
                        ++deletedRecordsIterator;
                    }
                    else
                    {
                        nextCheckpoint = changedRecordsIterator->originalIndex;
                        ++changedRecordsIterator;
                    }
                }
                else
                {
                    assert (deletedRecordsIterator != deletedRecordsEnd);
                    nextCheckpoint = *deletedRecordsIterator;
                    ++deletedRecordsIterator;
                }

                return nextCheckpoint;
            };

        std::size_t offset = 0u;
        std::size_t intervalBegin = AdvanceToNextCheckpoint ();
        std::size_t intervalEnd;

        auto OffsetInterval = [this, &intervalBegin, &intervalEnd, &offset] () -> void
        {
            std::size_t intervalSize = intervalEnd - intervalBegin - 1u;
            assert (intervalSize == 0u || intervalBegin + 1u < records.size ());

#ifndef NDEBUG
            // Interval begin can be out of bounds only if interval size is zero. It's ok, because memcpy skips
            // zero-size intervals. But debug version of std::vector throws out of bounds exception, therefore
            // we add this `if` in debug build. In release build it's skipped to improve performance.
            if (intervalBegin + 1u < records.size ())
            {
#endif
                memcpy (&records[intervalBegin - offset], &records[intervalBegin + 1u],
                        intervalSize * sizeof (void *));
#ifndef NDEBUG
            }
#endif

            ++offset;
        };

        while (changedRecordsIterator != changedRecordsEnd || deletedRecordsIterator != deletedRecordsEnd)
        {
            intervalEnd = AdvanceToNextCheckpoint ();
            OffsetInterval ();
            intervalBegin = intervalEnd;
        }

        intervalEnd = records.size ();
        OffsetInterval ();
        records.resize (intervalEnd - offset);
        deletedRecordIndices.clear ();
    }

    if (!changedRecords.empty ())
    {
        if (changedRecords.size () * Constants::OrderedIndex::MINIMUM_CHANGED_RECORDS_RATIO_TO_TRIGGER_FULL_RESORT >=
            records.size ())
        {
            MassInsertionExecutor executor = StartMassInsertion ();
            for (const ChangedRecordInfo &info : changedRecords)
            {
                executor.InsertRecord (info.record);
            }
        }
        else
        {
            for (const ChangedRecordInfo &info : changedRecords)
            {
                InsertRecord (info.record);
            }
        }

        changedRecords.clear ();
    }
}
} // namespace Emergence::Pegasus