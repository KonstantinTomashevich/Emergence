#include <algorithm>
#include <cassert>

#include <Pegasus/Constants/OrderedIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
/// Indicates that ::record could be changed and that ::backup should be used to get value, used to insert this record
/// into ordered vector. When record is changed using cursor from other index, we need to pass this structure instead
/// of just ::backup, because otherwise ::backup could be compared to changed record and that comparison will lead to
/// incorrect lookup result.
struct RecordWithBackup
{
    const void *record;
    const void *backup;
};

template <typename BaseComparator>
struct Comparator
{
    Comparator (const OrderedIndex *_index, BaseComparator _baseComparator) noexcept;

    bool operator() (const void *_firstRecord, const void *_secondRecord) const noexcept;

    bool operator() (const RecordWithBackup &_firstRecord, const void *_secondRecord) const noexcept;

    bool operator() (const void *_firstRecord, const RecordWithBackup &_secondRecord) const noexcept;

    bool operator() (const OrderedIndex::Bound &_bound, const void *_record) const noexcept;

    bool operator() (const void *_record, const OrderedIndex::Bound &_bound) const noexcept;

private:
    const void *GetValue (const void *_record) const noexcept;

    std::size_t fieldOffset;
    const BaseComparator baseComparator;
};

template <typename BaseComparator>
Comparator<BaseComparator>::Comparator (const OrderedIndex *_index, BaseComparator _baseComparator) noexcept
    : fieldOffset (_index->GetIndexedField ().GetOffset ()),
      baseComparator (std::move (_baseComparator))
{
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    assert (_firstRecord);
    assert (_secondRecord);
    return baseComparator.Compare (GetValue (_firstRecord), GetValue (_secondRecord)) < 0;
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const RecordWithBackup &_firstRecord,
                                             const void *_secondRecord) const noexcept
{
    assert (_secondRecord);
    if (_firstRecord.record == _secondRecord)
    {
        return false;
    }
    else
    {
        assert (_firstRecord.backup);
        return baseComparator.Compare (GetValue (_firstRecord.backup), GetValue (_secondRecord)) < 0;
    }
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const void *_firstRecord,
                                             const RecordWithBackup &_secondRecord) const noexcept
{
    assert (_firstRecord);
    if (_secondRecord.record == _firstRecord)
    {
        return false;
    }
    else
    {
        assert (_secondRecord.backup);
        return baseComparator.Compare (GetValue (_firstRecord), GetValue (_secondRecord.backup)) < 0;
    }
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const OrderedIndex::Bound &_bound, const void *_record) const noexcept
{
    assert (_bound.boundValue);
    assert (_record);
    return baseComparator.Compare (_bound.boundValue, GetValue (_record)) < 0;
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const void *_record, const OrderedIndex::Bound &_bound) const noexcept
{
    assert (_record);
    assert (_bound.boundValue);
    return baseComparator.Compare (GetValue (_record), _bound.boundValue) < 0;
}

template <typename BaseComparator>
const void *Comparator<BaseComparator>::GetValue (const void *_record) const noexcept
{
    return static_cast<const uint8_t *> (_record) + fieldOffset;
}

OrderedIndex::AscendingReadCursor::AscendingReadCursor (const OrderedIndex::AscendingReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::AscendingReadCursor::AscendingReadCursor (OrderedIndex::AscendingReadCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::AscendingReadCursor::~AscendingReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *OrderedIndex::AscendingReadCursor::operator* () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

OrderedIndex::AscendingReadCursor &OrderedIndex::AscendingReadCursor::operator++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

OrderedIndex::AscendingReadCursor::AscendingReadCursor (OrderedIndex *_index,
                                                        std::vector<const void *>::const_iterator _begin,
                                                        std::vector<const void *>::const_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::AscendingEditCursor::AscendingEditCursor (OrderedIndex::AscendingEditCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::AscendingEditCursor::~AscendingEditCursor () noexcept
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

void *OrderedIndex::AscendingEditCursor::operator* () noexcept
{
    assert (index);
    return current != end ? const_cast<void *> (*current) : nullptr;
}

OrderedIndex::AscendingEditCursor &OrderedIndex::AscendingEditCursor::operator~ () noexcept
{
    assert (index);
    assert (current != end);

    index->DeleteRecordMyself (current);
    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::AscendingEditCursor &OrderedIndex::AscendingEditCursor::operator++ () noexcept
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

OrderedIndex::AscendingEditCursor::AscendingEditCursor (OrderedIndex *_index,
                                                        std::vector<const void *>::iterator _begin,
                                                        std::vector<const void *>::iterator _end) noexcept
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

void OrderedIndex::AscendingEditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

OrderedIndex::DescendingReadCursor::DescendingReadCursor (const OrderedIndex::DescendingReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::DescendingReadCursor::DescendingReadCursor (OrderedIndex::DescendingReadCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::DescendingReadCursor::~DescendingReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *OrderedIndex::DescendingReadCursor::operator* () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

OrderedIndex::DescendingReadCursor &OrderedIndex::DescendingReadCursor::operator++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

OrderedIndex::DescendingReadCursor::DescendingReadCursor (
    OrderedIndex *_index,
    std::vector<const void *>::const_reverse_iterator _begin,
    std::vector<const void *>::const_reverse_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::DescendingEditCursor::DescendingEditCursor (OrderedIndex::DescendingEditCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::DescendingEditCursor::~DescendingEditCursor () noexcept
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

void *OrderedIndex::DescendingEditCursor::operator* () noexcept
{
    assert (index);
    return current != end ? const_cast<void *> (*current) : nullptr;
}

OrderedIndex::DescendingEditCursor &OrderedIndex::DescendingEditCursor::operator~ () noexcept
{
    assert (index);
    assert (current != end);

    index->DeleteRecordMyself (current);
    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::DescendingEditCursor &OrderedIndex::DescendingEditCursor::operator++ () noexcept
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

OrderedIndex::DescendingEditCursor::DescendingEditCursor (OrderedIndex *_index,
                                                          std::vector<const void *>::reverse_iterator _begin,
                                                          std::vector<const void *>::reverse_iterator _end) noexcept
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

void OrderedIndex::DescendingEditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

OrderedIndex::AscendingReadCursor OrderedIndex::LookupToReadAscending (const OrderedIndex::Bound &_min,
                                                                       const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::AscendingReadCursor (this, result.begin, result.end);
}

OrderedIndex::DescendingReadCursor OrderedIndex::LookupToReadDescending (const OrderedIndex::Bound &_min,
                                                                         const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::DescendingReadCursor (this, std::vector<const void *>::const_reverse_iterator (result.end),
                                               std::vector<const void *>::const_reverse_iterator (result.begin));
}

OrderedIndex::AscendingEditCursor OrderedIndex::LookupToEditAscending (const OrderedIndex::Bound &_min,
                                                                       const OrderedIndex::Bound &_max) noexcept
{
    hasEditCursor = true;
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::AscendingEditCursor (this, result.begin, result.end);
}

OrderedIndex::DescendingEditCursor OrderedIndex::LookupToEditDescending (const OrderedIndex::Bound &_min,
                                                                         const OrderedIndex::Bound &_max) noexcept
{
    hasEditCursor = true;
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::DescendingEditCursor (this, std::vector<const void *>::reverse_iterator (result.end),
                                               std::vector<const void *>::reverse_iterator (result.begin));
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

OrderedIndex::MassInsertionExecutor::~MassInsertionExecutor () noexcept
{
    assert (owner);
    DoWithCorrectComparator (owner->indexedField,
                             [this] (auto _comparator) -> void
                             {
                                 std::sort (owner->records.begin (), owner->records.end (),
                                            Comparator (owner, _comparator));
                             });

#ifndef NDEBUG
    assert (owner->massInsertionInProgress);
    owner->massInsertionInProgress = false;
#endif
}

void OrderedIndex::MassInsertionExecutor::InsertRecord (const void *_record) noexcept
{
    assert (owner);
    assert (_record);
    owner->records.emplace_back (_record);
}

OrderedIndex::MassInsertionExecutor::MassInsertionExecutor (OrderedIndex *_owner) noexcept : owner (_owner)
{
    assert (owner);

#ifndef NDEBUG
    assert (!owner->massInsertionInProgress);
    owner->massInsertionInProgress = true;
#endif
}

OrderedIndex::OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField)
    : IndexBase (_owner),
      indexedField (_owner->GetRecordMapping ().GetField (_indexedField))
{
    assert (indexedField.IsHandleValid ());
}

OrderedIndex::InternalLookupResult OrderedIndex::InternalLookup (const OrderedIndex::Bound &_min,
                                                                 const OrderedIndex::Bound &_max) noexcept
{
    return DoWithCorrectComparator (
        indexedField,
        [this, &_min, &_max] (auto _comparator)
        {
            assert (!_min.boundValue || !_max.boundValue ||
                    _comparator.Compare (_min.boundValue, _max.boundValue) <= 0);
            InternalLookupResult result {records.begin (), records.end ()};

            if (_min.boundValue)
            {
                result.begin =
                    std::lower_bound (records.begin (), records.end (), _min, Comparator (this, _comparator));
            }

            if (_max.boundValue)
            {
                result.end = std::upper_bound (result.begin, records.end (), _max, Comparator (this, _comparator));
            }

            return result;
        });
}

std::vector<const void *>::const_iterator OrderedIndex::LocateRecord (const void *_record,
                                                                      const void *_recordBackup) const noexcept
{
    auto iterator = DoWithCorrectComparator (indexedField,
                                             [this, _record, _recordBackup] (auto _comparator)
                                             {
                                                 return std::lower_bound (records.begin (), records.end (),
                                                                          RecordWithBackup {_record, _recordBackup},
                                                                          Comparator (this, _comparator));
                                             });

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
    assert (_record);
    DoWithCorrectComparator (indexedField,
                             [this, _record] (auto _comparator)
                             {
                                 auto place = std::upper_bound (records.begin (), records.end (), _record,
                                                                Comparator (this, _comparator));
                                 records.insert (place, _record);
                             });
}

OrderedIndex::MassInsertionExecutor OrderedIndex::StartMassInsertion () noexcept
{
    return MassInsertionExecutor (this);
}

void OrderedIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    assert (!hasEditCursor);
    auto iterator = LocateRecord (_record, _recordBackup);
    assert (iterator != records.end ());
    records.erase (iterator);
}

void OrderedIndex::DeleteRecordMyself (const std::vector<const void *>::iterator &_position) noexcept
{
    assert (_position != records.end ());
    std::size_t index = _position - records.begin ();

    assert (deletedRecordIndices.empty () || index > deletedRecordIndices.back ());
    deletedRecordIndices.emplace_back (index);
    storage->DeleteRecord (const_cast<void *> (*_position), this);
}

void OrderedIndex::DeleteRecordMyself (const std::vector<const void *>::reverse_iterator &_position) noexcept
{
    assert (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;

    assert (deletedRecordIndices.empty () || index < deletedRecordIndices.front ());
    deletedRecordIndices.emplace (deletedRecordIndices.begin (), index);
    storage->DeleteRecord (const_cast<void *> (*_position), this);
}

void OrderedIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    assert (!hasEditCursor);
    auto iterator = LocateRecord (_record, _recordBackup);
    assert (iterator != records.end ());

    changedRecords.emplace_back (ChangedRecordInfo {std::numeric_limits<std::size_t>::max (), (_record)});
    records.erase (iterator);
}

void OrderedIndex::OnRecordChangedByMe (const std::vector<const void *>::iterator &_position) noexcept
{
    assert (_position != records.end ());
    std::size_t index = _position - records.begin ();

    assert (changedRecords.empty () || index > changedRecords.back ().originalIndex);
    changedRecords.emplace_back (ChangedRecordInfo {index, *_position});
}

void OrderedIndex::OnRecordChangedByMe (const std::vector<const void *>::reverse_iterator &_position) noexcept
{
    assert (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;
    assert (changedRecords.empty () || index < changedRecords.front ().originalIndex);
    changedRecords.emplace (changedRecords.begin (), ChangedRecordInfo {index, *_position});
}

void OrderedIndex::OnWriterClosed () noexcept
{
    if (hasEditCursor && (!changedRecords.empty () || !deletedRecordIndices.empty ()))
    {
        auto changedRecordsIterator = changedRecords.begin ();
        const auto changedRecordsEnd = changedRecords.end ();

        auto deletedRecordsIterator = deletedRecordIndices.begin ();
        const auto deletedRecordsEnd = deletedRecordIndices.end ();

        auto AdvanceToNextCheckpoint = [&changedRecordsIterator, &changedRecordsEnd, &deletedRecordsIterator,
                                        &deletedRecordsEnd] () -> std::size_t
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
                memcpy (&records[intervalBegin - offset], &records[intervalBegin + 1u], intervalSize * sizeof (void *));
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
        if (changedRecords.size () >= static_cast<float> (changedRecords.size () + records.size ()) *
                                          Constants::OrderedIndex::MINIMUM_CHANGED_RECORDS_RATIO_TO_TRIGGER_FULL_RESORT)
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

    hasEditCursor = false;
}
} // namespace Emergence::Pegasus