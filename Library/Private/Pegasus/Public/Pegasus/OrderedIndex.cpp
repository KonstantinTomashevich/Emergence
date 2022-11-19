#include <algorithm>
#include <cstring>

#include <Memory/Profiler/AllocationGroup.hpp>

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
    EMERGENCE_ASSERT (_firstRecord);
    EMERGENCE_ASSERT (_secondRecord);
    return baseComparator.Compare (GetValue (_firstRecord), GetValue (_secondRecord)) < 0;
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const RecordWithBackup &_firstRecord,
                                             const void *_secondRecord) const noexcept
{
    EMERGENCE_ASSERT (_secondRecord);
    if (_firstRecord.record == _secondRecord)
    {
        return false;
    }
    else
    {
        EMERGENCE_ASSERT (_firstRecord.backup);
        return baseComparator.Compare (GetValue (_firstRecord.backup), GetValue (_secondRecord)) < 0;
    }
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const void *_firstRecord,
                                             const RecordWithBackup &_secondRecord) const noexcept
{
    EMERGENCE_ASSERT (_firstRecord);
    if (_secondRecord.record == _firstRecord)
    {
        return false;
    }

    EMERGENCE_ASSERT (_secondRecord.backup);
    return baseComparator.Compare (GetValue (_firstRecord), GetValue (_secondRecord.backup)) < 0;
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const OrderedIndex::Bound &_bound, const void *_record) const noexcept
{
    EMERGENCE_ASSERT (_bound.boundValue);
    EMERGENCE_ASSERT (_record);
    return baseComparator.Compare (_bound.boundValue, GetValue (_record)) < 0;
}

template <typename BaseComparator>
bool Comparator<BaseComparator>::operator() (const void *_record, const OrderedIndex::Bound &_bound) const noexcept
{
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (_bound.boundValue);
    return baseComparator.Compare (GetValue (_record), _bound.boundValue) < 0;
}

template <typename BaseComparator>
const void *Comparator<BaseComparator>::GetValue (const void *_record) const noexcept
{
    return static_cast<const uint8_t *> (_record) + fieldOffset;
}

#define READ_CURSOR_IMPLEMENTATION(Cursor, Iterator)                                                                   \
    OrderedIndex::Cursor::Cursor (const OrderedIndex::Cursor &_other) noexcept                                         \
        : index (_other.index),                                                                                        \
          current (_other.current),                                                                                    \
          end (_other.end)                                                                                             \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        ++index->activeCursors;                                                                                        \
        index->storage->RegisterReader ();                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor::Cursor (OrderedIndex::Cursor &&_other) noexcept                                              \
        : index (_other.index),                                                                                        \
          current (_other.current),                                                                                    \
          end (_other.end)                                                                                             \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        _other.index = nullptr;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor::~Cursor () noexcept                                                                          \
    {                                                                                                                  \
        if (index)                                                                                                     \
        {                                                                                                              \
            --index->activeCursors;                                                                                    \
            index->storage->UnregisterReader ();                                                                       \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    const void *OrderedIndex::Cursor::operator* () const noexcept                                                      \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        return current != end ? *current : nullptr;                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor &OrderedIndex::Cursor::operator++ () noexcept                                                 \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        EMERGENCE_ASSERT (current != end);                                                                             \
                                                                                                                       \
        ++current;                                                                                                     \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    OrderedIndex::Cursor::Cursor (OrderedIndex *_index, Iterator _begin, Iterator _end) noexcept                       \
        : index (_index),                                                                                              \
          current (_begin),                                                                                            \
          end (_end)                                                                                                   \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        EMERGENCE_ASSERT (current <= end);                                                                             \
                                                                                                                       \
        ++index->activeCursors;                                                                                        \
        index->storage->RegisterReader ();                                                                             \
    }

#define EDIT_CURSOR_IMPLEMENTATION(Cursor, Iterator)                                                                   \
    OrderedIndex::Cursor::Cursor (OrderedIndex::Cursor &&_other) noexcept                                              \
        : index (_other.index),                                                                                        \
          current (_other.current),                                                                                    \
          end (_other.end)                                                                                             \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        _other.index = nullptr;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor::~Cursor () noexcept                                                                          \
    {                                                                                                                  \
        if (index)                                                                                                     \
        {                                                                                                              \
            if (current != end && index->storage->EndRecordEdition (*current, index))                                  \
            {                                                                                                          \
                index->OnRecordChangedByMe (current);                                                                  \
            }                                                                                                          \
                                                                                                                       \
            --index->activeCursors;                                                                                    \
            index->storage->UnregisterWriter ();                                                                       \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void *OrderedIndex::Cursor::operator* () noexcept                                                                  \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        return current != end ? const_cast<void *> (*current) : nullptr;                                               \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor &OrderedIndex::Cursor::operator~() noexcept                                                   \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        EMERGENCE_ASSERT (current != end);                                                                             \
                                                                                                                       \
        index->DeleteRecordMyself (current);                                                                           \
        ++current;                                                                                                     \
        BeginRecordEdition ();                                                                                         \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    OrderedIndex::Cursor &OrderedIndex::Cursor::operator++ () noexcept                                                 \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        EMERGENCE_ASSERT (current != end);                                                                             \
                                                                                                                       \
        if (index->storage->EndRecordEdition (*current, index))                                                        \
        {                                                                                                              \
            index->OnRecordChangedByMe (current);                                                                      \
        }                                                                                                              \
                                                                                                                       \
        ++current;                                                                                                     \
        BeginRecordEdition ();                                                                                         \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): Types can not be enclosed. */                                       \
    OrderedIndex::Cursor::Cursor (OrderedIndex *_index, Iterator _begin, Iterator _end) noexcept                       \
        : index (_index),                                                                                              \
          current (_begin),                                                                                            \
          end (_end)                                                                                                   \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        EMERGENCE_ASSERT (current <= end);                                                                             \
                                                                                                                       \
        ++index->activeCursors;                                                                                        \
        index->storage->RegisterWriter ();                                                                             \
        BeginRecordEdition ();                                                                                         \
    }                                                                                                                  \
                                                                                                                       \
    void OrderedIndex::Cursor::BeginRecordEdition () const noexcept                                                    \
    {                                                                                                                  \
        EMERGENCE_ASSERT (index);                                                                                      \
        if (current != end)                                                                                            \
        {                                                                                                              \
            index->storage->BeginRecordEdition (*current);                                                             \
        }                                                                                                              \
    }

READ_CURSOR_IMPLEMENTATION (AscendingReadCursor, Container::Vector<const void *>::const_iterator)

EDIT_CURSOR_IMPLEMENTATION (AscendingEditCursor, Container::Vector<const void *>::iterator)

READ_CURSOR_IMPLEMENTATION (DescendingReadCursor, Container::Vector<const void *>::const_reverse_iterator)

EDIT_CURSOR_IMPLEMENTATION (DescendingEditCursor, Container::Vector<const void *>::reverse_iterator)

OrderedIndex::AscendingReadCursor OrderedIndex::LookupToReadAscending (const OrderedIndex::Bound &_min,
                                                                       const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return {this, result.begin, result.end};
}

OrderedIndex::DescendingReadCursor OrderedIndex::LookupToReadDescending (const OrderedIndex::Bound &_min,
                                                                         const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return {this, Container::Vector<const void *>::const_reverse_iterator (result.end),
            Container::Vector<const void *>::const_reverse_iterator (result.begin)};
}

OrderedIndex::AscendingEditCursor OrderedIndex::LookupToEditAscending (const OrderedIndex::Bound &_min,
                                                                       const OrderedIndex::Bound &_max) noexcept
{
    hasEditCursor = true;
    InternalLookupResult result = InternalLookup (_min, _max);
    return {this, result.begin, result.end};
}

OrderedIndex::DescendingEditCursor OrderedIndex::LookupToEditDescending (const OrderedIndex::Bound &_min,
                                                                         const OrderedIndex::Bound &_max) noexcept
{
    hasEditCursor = true;
    InternalLookupResult result = InternalLookup (_min, _max);
    return {this, Container::Vector<const void *>::reverse_iterator (result.end),
            Container::Vector<const void *>::reverse_iterator (result.begin)};
}

StandardLayout::Field OrderedIndex::GetIndexedField () const noexcept
{
    return indexedField;
}

void OrderedIndex::Drop () noexcept
{
    EMERGENCE_ASSERT (CanBeDropped ());
    EMERGENCE_ASSERT (storage);
    storage->DropIndex (*this);
}

OrderedIndex::MassInsertionExecutor::~MassInsertionExecutor () noexcept
{
    EMERGENCE_ASSERT (owner);
    DoWithCorrectComparator (owner->indexedField,
                             [this] (auto _comparator) -> void
                             {
                                 std::sort (owner->records.begin (), owner->records.end (),
                                            Comparator (owner, _comparator));
                             });

#ifndef NDEBUG
    EMERGENCE_ASSERT (owner->massInsertionInProgress);
    owner->massInsertionInProgress = false;
#endif
}

void OrderedIndex::MassInsertionExecutor::InsertRecord (const void *_record) noexcept
{
    EMERGENCE_ASSERT (owner);
    EMERGENCE_ASSERT (_record);
    owner->records.emplace_back (_record);
}

OrderedIndex::MassInsertionExecutor::MassInsertionExecutor (OrderedIndex *_owner) noexcept
    : owner (_owner)
{
    EMERGENCE_ASSERT (owner);

#ifndef NDEBUG
    EMERGENCE_ASSERT (!owner->massInsertionInProgress);
    owner->massInsertionInProgress = true;
#endif
}

using namespace Memory::Literals;

OrderedIndex::OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField)
    : IndexBase (_owner),
      indexedField (_owner->GetRecordMapping ().GetField (_indexedField)),
      records (Memory::Profiler::AllocationGroup {"Ordering"_us}),
      changedRecords (Memory::Profiler::AllocationGroup {"ChangedRecords"_us}),
      deletedRecordIndices (Memory::Profiler::AllocationGroup {"DeletedIndices"_us})
{
    EMERGENCE_ASSERT (indexedField.IsHandleValid ());
}

OrderedIndex::InternalLookupResult OrderedIndex::InternalLookup (const OrderedIndex::Bound &_min,
                                                                 const OrderedIndex::Bound &_max) noexcept
{
    return DoWithCorrectComparator (
        indexedField,
        [this, &_min, &_max] (auto _comparator)
        {
            EMERGENCE_ASSERT (!_min.boundValue || !_max.boundValue ||
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

Container::Vector<const void *>::const_iterator OrderedIndex::LocateRecord (const void *_record,
                                                                            const void *_recordBackup) const noexcept
{
    auto iterator = DoWithCorrectComparator (indexedField,
                                             [this, _record, _recordBackup] (auto _comparator)
                                             {
                                                 return std::lower_bound (records.begin (), records.end (),
                                                                          RecordWithBackup {_record, _recordBackup},
                                                                          Comparator (this, _comparator));
                                             });

    EMERGENCE_ASSERT (iterator != records.end ());

    while (*iterator != _record)
    {
        ++iterator;
        EMERGENCE_ASSERT (iterator != records.end ());
    }

    return iterator;
}

void OrderedIndex::InsertRecord (const void *_record) noexcept
{
    EMERGENCE_ASSERT (_record);
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
    EMERGENCE_ASSERT (!hasEditCursor);
    auto iterator = LocateRecord (_record, _recordBackup);
    EMERGENCE_ASSERT (iterator != records.end ());
    records.erase (iterator);
}

void OrderedIndex::DeleteRecordMyself (const Container::Vector<const void *>::iterator &_position) noexcept
{
    EMERGENCE_ASSERT (_position != records.end ());
    std::size_t index = _position - records.begin ();

    EMERGENCE_ASSERT (deletedRecordIndices.empty () || index > deletedRecordIndices.back ());
    deletedRecordIndices.emplace_back (index);
    storage->DeleteRecord (const_cast<void *> (*_position), this);
}

void OrderedIndex::DeleteRecordMyself (const Container::Vector<const void *>::reverse_iterator &_position) noexcept
{
    EMERGENCE_ASSERT (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;

    EMERGENCE_ASSERT (deletedRecordIndices.empty () || index < deletedRecordIndices.front ());
    deletedRecordIndices.emplace (deletedRecordIndices.begin (), index);
    storage->DeleteRecord (const_cast<void *> (*_position), this);
}

void OrderedIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    EMERGENCE_ASSERT (!hasEditCursor);
    auto iterator = LocateRecord (_record, _recordBackup);
    EMERGENCE_ASSERT (iterator != records.end ());

    changedRecords.emplace_back (ChangedRecordInfo {std::numeric_limits<std::size_t>::max (), (_record)});
    records.erase (iterator);
}

void OrderedIndex::OnRecordChangedByMe (const Container::Vector<const void *>::iterator &_position) noexcept
{
    EMERGENCE_ASSERT (_position != records.end ());
    std::size_t index = _position - records.begin ();

    EMERGENCE_ASSERT (changedRecords.empty () || index > changedRecords.back ().originalIndex);
    changedRecords.emplace_back (ChangedRecordInfo {index, *_position});
}

void OrderedIndex::OnRecordChangedByMe (const Container::Vector<const void *>::reverse_iterator &_position) noexcept
{
    EMERGENCE_ASSERT (_position != records.rend ());
    std::size_t index = records.rend () - _position - 1u;
    EMERGENCE_ASSERT (changedRecords.empty () || index < changedRecords.front ().originalIndex);
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

        auto advanceToNextCheckpoint = [&changedRecordsIterator, &changedRecordsEnd, &deletedRecordsIterator,
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
                EMERGENCE_ASSERT (deletedRecordsIterator != deletedRecordsEnd);
                nextCheckpoint = *deletedRecordsIterator;
                ++deletedRecordsIterator;
            }

            return nextCheckpoint;
        };

        std::size_t offset = 0u;
        std::size_t intervalBegin = advanceToNextCheckpoint ();
        std::size_t intervalEnd;

        auto offsetInterval = [this, &intervalBegin, &intervalEnd, &offset] () -> void
        {
            std::size_t intervalSize = intervalEnd - intervalBegin - 1u;
            EMERGENCE_ASSERT (intervalSize == 0u || intervalBegin + 1u < records.size ());

#ifndef NDEBUG
            // Interval begin can be out of bounds only if interval size is zero. It's ok, because memcpy skips
            // zero-size intervals. But debug version of Container::Vector throws out of bounds exception, therefore
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
            intervalEnd = advanceToNextCheckpoint ();
            offsetInterval ();
            intervalBegin = intervalEnd;
        }

        intervalEnd = records.size ();
        offsetInterval ();
        records.resize (intervalEnd - offset);
        deletedRecordIndices.clear ();
    }

    if (!changedRecords.empty ())
    {
        if (static_cast<float> (changedRecords.size ()) >=
            static_cast<float> (changedRecords.size () + records.size ()) *
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
