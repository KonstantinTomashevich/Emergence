#include <Hashing/ByteHasher.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

#include <Pegasus/HashIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
const HashIndex::IndexedFieldVector &HashIndex::GetIndexedFields () const noexcept
{
    return indexedFields;
}

void HashIndex::Drop () noexcept
{
    EMERGENCE_ASSERT (CanBeDropped ());
    EMERGENCE_ASSERT (storage);
    storage->DropIndex (*this);
}

static size_t CalculateMask (const StandardLayout::Field &_field)
{
    EMERGENCE_ASSERT (_field.GetSize () <= sizeof (size_t));
    std::array<uint8_t, sizeof (size_t)> byteRepresentation;
    std::fill (byteRepresentation.begin (), byteRepresentation.end (), uint8_t (0u));

    if (_field.GetArchetype () == StandardLayout::FieldArchetype::BIT)
    {
        byteRepresentation[0u] = 1u << _field.GetBitOffset ();
    }
    else
    {
        for (size_t index = 0u; index < _field.GetSize (); ++index)
        {
            byteRepresentation[index] = 255u;
        }
    }

    return *reinterpret_cast<size_t *> (&byteRepresentation.front ());
}

inline size_t ExtractFromRecord (const void *_record, size_t _mask, size_t _offset)
{
    return _mask & *reinterpret_cast<const size_t *> (static_cast<const uint8_t *> (_record) + _offset);
}

inline size_t ExtractFromLookup (const void *_lookup, size_t _mask)
{
    return _mask & *static_cast<const size_t *> (_lookup);
}

HashIndex::DirectHasher::DirectHasher (HashIndex *_owner) noexcept
    : mask (CalculateMask (_owner->GetIndexedFields ().Front ())),
      offset (_owner->GetIndexedFields ().Front ().GetOffset ())
{
}

std::size_t HashIndex::DirectHasher::operator() (const void *_record) const noexcept
{
    return ExtractFromRecord (_record, mask, offset);
}

std::size_t HashIndex::DirectHasher::operator() (const HashIndex::RecordWithBackup &_record) const noexcept
{
    return (*this) (_record.backup);
}

std::size_t HashIndex::DirectHasher::operator() (const HashIndex::LookupRequest &_request) const noexcept
{
    return ExtractFromLookup (_request.indexedFieldValues, mask);
}

HashIndex::DirectComparator::DirectComparator (HashIndex *_owner) noexcept
    : mask (CalculateMask (_owner->GetIndexedFields ().Front ())),
      offset (_owner->GetIndexedFields ().Front ().GetOffset ())
{
}

bool HashIndex::DirectComparator::operator() (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    return _firstRecord == _secondRecord;
}

bool HashIndex::DirectComparator::operator() (const void *_record,
                                              const HashIndex::RecordWithBackup &_recordWithBackup) const noexcept
{
    return _record == _recordWithBackup.record;
}

bool HashIndex::DirectComparator::operator() (const void *_record,
                                              const HashIndex::LookupRequest &_request) const noexcept
{
    // Theoretically, we can modify offset and mask to make sure that size_t pointer will always be aligned.
    // It would speed up record value extraction, but it would also decrease lookup value extraction. In most
    // situations, direct hashing is used with size_t variables, therefore alignment is always correct, it means that
    // there is no practical sense to modify offset and mask as it would not change general performance drastically.
    return ExtractFromRecord (_record, mask, offset) == ExtractFromLookup (_request.indexedFieldValues, mask);
}

bool HashIndex::DirectComparator::operator() (const HashIndex::LookupRequest &_request,
                                              const void *_record) const noexcept
{
    return (*this) (_record, _request);
}

static void UpdateHash (Hashing::ByteHasher &_hasher, const StandardLayout::Field &_indexedField, const uint8_t *_value)
{
    switch (_indexedField.GetArchetype ())
    {
    case StandardLayout::FieldArchetype::INT:
    case StandardLayout::FieldArchetype::UINT:
    case StandardLayout::FieldArchetype::FLOAT:
    case StandardLayout::FieldArchetype::BLOCK:
    {
        _hasher.Append (_value, _indexedField.GetSize ());
        break;
    }

    case StandardLayout::FieldArchetype::BIT:
    {
        // Currently, we hash bits as bytes that could have 1 only on given bit offset.
        uint8_t mask = 1u << _indexedField.GetBitOffset ();

        if (*_value & mask)
        {
            _hasher.Append (mask);
        }
        else
        {
            _hasher.Append (0u);
        }

        break;
    }

    case StandardLayout::FieldArchetype::STRING:
    {
        const auto *current = _value;
        const uint8_t *stringEnd = current + _indexedField.GetSize ();

        while (*current && current != stringEnd)
        {
            _hasher.Append (*current);
            ++current;
        }

        break;
    }

    case StandardLayout::FieldArchetype::UNIQUE_STRING:
    {
        const uintptr_t hash = reinterpret_cast<const Memory::UniqueString *> (_value)->Hash ();
        _hasher.Append (reinterpret_cast<const uint8_t *> (&hash), sizeof (hash));
        break;
    }

    case StandardLayout::FieldArchetype::NESTED_OBJECT:
    case StandardLayout::FieldArchetype::VECTOR:
    case StandardLayout::FieldArchetype::PATCH:
        // Unsupported field archetypes.
        EMERGENCE_ASSERT (false);
        break;
    }
}

std::size_t HashIndex::GenericHasher::operator() (const void *_record) const noexcept
{
    EMERGENCE_ASSERT (owner);
    EMERGENCE_ASSERT (_record);

    Hashing::ByteHasher hasher;
    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        UpdateHash (hasher, indexedField, static_cast<const uint8_t *> (indexedField.GetValue (_record)));
    }

    return hasher.GetCurrentValue () % std::numeric_limits<std::size_t>::max ();
}

std::size_t HashIndex::GenericHasher::operator() (const HashIndex::RecordWithBackup &_record) const noexcept
{
    return (*this) (_record.backup);
}

std::size_t HashIndex::GenericHasher::operator() (const HashIndex::LookupRequest &_request) const noexcept
{
    EMERGENCE_ASSERT (owner);
    EMERGENCE_ASSERT (_request.indexedFieldValues);

    Hashing::ByteHasher hasher;
    const auto *currentFieldBegin = static_cast<const uint8_t *> (_request.indexedFieldValues);

    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        UpdateHash (hasher, indexedField, currentFieldBegin);
        currentFieldBegin += indexedField.GetSize ();
    }

    return hasher.GetCurrentValue () % std::numeric_limits<std::size_t>::max ();
}

bool HashIndex::GenericComparator::operator() (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    return _firstRecord == _secondRecord;
}

bool HashIndex::GenericComparator::operator() (const void *_record,
                                               const HashIndex::RecordWithBackup &_recordWithBackup) const noexcept
{
    return _record == _recordWithBackup.record;
}

bool HashIndex::GenericComparator::operator() (const void *_record,
                                               const HashIndex::LookupRequest &_request) const noexcept
{
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (_request.indexedFieldValues);

    const auto *currentFieldBegin = static_cast<const uint8_t *> (_request.indexedFieldValues);

    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        EMERGENCE_ASSERT (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreFieldValuesEqual (indexedField.GetValue (_record), currentFieldBegin, indexedField))
        {
            return false;
        }

        currentFieldBegin += indexedField.GetSize ();
    }

    return true;
}

bool HashIndex::GenericComparator::operator() (const HashIndex::LookupRequest &_request,
                                               const void *_record) const noexcept
{
    return (*this) (_record, _request);
}

using namespace Memory::Literals;

HashIndex::HashIndex (Storage *_owner,
                      std::size_t _initialBuckets,
                      const Container::Vector<StandardLayout::FieldId> &_indexedFields)
    : IndexBase (_owner),
      changedNodes (Memory::Profiler::AllocationGroup {"ChangedNodes"_us})
{
    EMERGENCE_ASSERT (!_indexedFields.empty ());
    EMERGENCE_ASSERT (_indexedFields.size () < Constants::HashIndex::MAX_INDEXED_FIELDS);

    std::size_t indexedFieldsCount = std::min (Constants::HashIndex::MAX_INDEXED_FIELDS, _indexedFields.size ());
    const StandardLayout::Mapping &recordMapping = storage->GetRecordMapping ();

    for (std::size_t index = 0u; index < indexedFieldsCount; ++index)
    {
        StandardLayout::Field indexedField = recordMapping.GetField (_indexedFields[index]);
        EMERGENCE_ASSERT (indexedField.IsHandleValid ());
        indexedFields.EmplaceBack (indexedField);
    }

    if (indexedFields.GetCount () == 1u && indexedFields.Front ().GetSize () <= sizeof (size_t))
    {
        implementationSwitch = 0u;
        new (&records.direct) DirectHashSet {_initialBuckets, DirectHasher {this}, DirectComparator {this},
                                             Memory::Profiler::AllocationGroup {"MultiSet"_us}};
    }
    else
    {
        implementationSwitch = 1u;
        new (&records.generic) GenericHashSet {_initialBuckets, GenericHasher {this}, GenericComparator {this},
                                               Memory::Profiler::AllocationGroup {"MultiSet"_us}};
    }
}

HashIndex::~HashIndex () noexcept
{
    EMERGENCE_UNION2_DESTRUCT (records, implementationSwitch);
}

void HashIndex::InsertRecord (const void *_record) noexcept
{
    EMERGENCE_UNION2_CALL (records, implementationSwitch, void, emplace, _record);
}

void HashIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    VisitUnion2<void> (
        [_record, _recordBackup] (auto &_records)
        {
            auto iterator = _records.find (RecordWithBackup {_record, _recordBackup});
            EMERGENCE_ASSERT (iterator != _records.end ());

            // To erase record using iterator unordered multiset must calculate hash once more.
            // But record, to which iterator points, could be changed, therefore hash could be incorrect.
            // We forcefully rewrite pointed value with backup to ensure that computed has will be correct.
            // This adhok could be eliminated by addition of alternative queries support (like find with
            // RecordWithBackup) to erase operation.
            const_cast<void const *&> (*iterator) = _recordBackup;

            _records.erase (iterator);
        },
        records, implementationSwitch);
}

#define DELETE_RECORD_MYSELF(SwitchId)                                                                                 \
    HashIndex::RecordHashSetIterator HashIndex::DeleteRecordMyself##SwitchId (                                         \
        const RecordHashSetIterator &_position) noexcept                                                               \
    {                                                                                                                  \
        EMERGENCE_ASSERT (_position != records._##SwitchId.end ());                                                    \
        const void *record = *_position;                                                                               \
                                                                                                                       \
        /* To erase record using iterator unordered multiset must calculate hash once more.                            \
           But record, to which iterator points, could be changed, therefore hash could be incorrect.                  \
           We forcefully rewrite pointed value with backup to ensure that computed has will be correct.                \
           This adhok could be eliminated by addition of alternative queries support (like find with                   \
           RecordWithBackup) to erase operation. */                                                                    \
        const_cast<void const *&> (*_position) = storage->GetEditedRecordBackup ();                                    \
                                                                                                                       \
        auto next = records._##SwitchId.erase (_position);                                                             \
        storage->DeleteRecord (const_cast<void *> (record), this);                                                     \
        return next;                                                                                                   \
    }

DELETE_RECORD_MYSELF (0)
DELETE_RECORD_MYSELF (1)
#undef DELETE_RECORD_MYSELF

void HashIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    VisitUnion2<void> (
        [this, _record, _recordBackup] (auto &_records)
        {
            auto iterator = _records.find (RecordWithBackup {_record, _recordBackup});
            EMERGENCE_ASSERT (iterator != _records.end ());

            // To extract record using iterator unordered multiset must calculate hash once more.
            // But record, to which iterator points, could be changed, therefore hash could be incorrect.
            // We forcefully rewrite pointed value with backup to ensure that computed has will be correct.
            // This adhok could be eliminated by addition of alternative queries support (like find with
            // RecordWithBackup) to extract operation.
            const_cast<void const *&> (*iterator) = _recordBackup;

            // After extraction, we must restore node value, because this node will be reinserted later.
            changedNodes.emplace_back (_records.extract (iterator)).value () = _record;
        },
        records, implementationSwitch);
}

#define ON_RECORD_CHANGED_BY_ME(SwitchId)                                                                              \
    void HashIndex::OnRecordChangedByMe##SwitchId (RecordHashSetIterator _position) noexcept                           \
    {                                                                                                                  \
        const void *record = *_position;                                                                               \
        /* To extract record using iterator unordered multiset must calculate hash once more.                          \
           But record, to which iterator points, could be changed, therefore hash could be incorrect.                  \
           We forcefully rewrite pointed value with backup to ensure that computed has will be correct.                \
           This adhok could be eliminated by addition of alternative queries support (like find with                   \
           RecordWithBackup) to extract operation. */                                                                  \
        const_cast<void const *&> (*_position) = storage->GetEditedRecordBackup ();                                    \
                                                                                                                       \
        /* After extraction we must restore node value, because this node will be reinserted later. */                 \
        changedNodes.emplace_back (records._##SwitchId.extract (_position)).value () = record;                         \
    }

ON_RECORD_CHANGED_BY_ME (0)
ON_RECORD_CHANGED_BY_ME (1)
#undef ON_RECORD_CHANGED_BY_ME

void HashIndex::OnWriterClosed () noexcept
{
    // Reinsert extracted changed nodes back into index.
    VisitUnion2<void> (
        [this] (auto &_records)
        {
            for (auto &changedNode : changedNodes)
            {
                EMERGENCE_ASSERT (changedNode);
                _records.insert (std::move (changedNode));
            }
        },
        records, implementationSwitch);

    changedNodes.clear ();
}

HashIndex::ReadCursor::ReadCursor (const HashIndex::ReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

HashIndex::ReadCursor::ReadCursor (HashIndex::ReadCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    EMERGENCE_ASSERT (index);
    _other.index = nullptr;
}

HashIndex::ReadCursor::~ReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *HashIndex::ReadCursor::operator* () const noexcept
{
    EMERGENCE_ASSERT (index);
    return current != end ? *current : nullptr;
}

HashIndex::ReadCursor &HashIndex::ReadCursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != end);

    ++current;
    return *this;
}

HashIndex::ReadCursor::ReadCursor (HashIndex *_index,
                                   RecordHashSetConstIterator _begin,
                                   RecordHashSetConstIterator _end) noexcept
    : index (_index),
      current (_begin),
      end (_end)
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

HashIndex::EditCursor::EditCursor (HashIndex::EditCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    EMERGENCE_ASSERT (index);
    _other.index = nullptr;
}

HashIndex::EditCursor::~EditCursor () noexcept
{
    if (index)
    {
        if (current != end && index->storage->EndRecordEdition (*current, index))
        {
            EMERGENCE_UNION2_SWITCH_CALL (index->implementationSwitch, index->OnRecordChangedByMe, current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *HashIndex::EditCursor::operator* () noexcept
{
    EMERGENCE_ASSERT (index);
    return current != end ? const_cast<void *> (*current) : nullptr;
}

HashIndex::EditCursor &HashIndex::EditCursor::operator~() noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != end);

    current = EMERGENCE_UNION2_SWITCH_CALL (index->implementationSwitch, index->DeleteRecordMyself, current);
    BeginRecordEdition ();
    return *this;
}

HashIndex::EditCursor &HashIndex::EditCursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != end);

    const void *record = *current;
    auto previous = current++;

    if (index->storage->EndRecordEdition (record, index))
    {
        EMERGENCE_UNION2_SWITCH_CALL (index->implementationSwitch, index->OnRecordChangedByMe, previous);
    }

    BeginRecordEdition ();
    return *this;
}

HashIndex::EditCursor::EditCursor (HashIndex *_index,
                                   RecordHashSetConstIterator _begin,
                                   RecordHashSetConstIterator _end) noexcept
    : index (_index),
      current (_begin),
      end (_end)
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void HashIndex::EditCursor::BeginRecordEdition () const noexcept
{
    EMERGENCE_ASSERT (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

HashIndex::ReadCursor HashIndex::LookupToRead (const HashIndex::LookupRequest &_request) noexcept
{
    using IteratorPair = std::pair<RecordHashSetConstIterator, RecordHashSetConstIterator>;
    auto [begin, end] = EMERGENCE_UNION2_CALL (records, implementationSwitch, IteratorPair, equal_range, _request);
    return {this, begin, end};
}

HashIndex::EditCursor HashIndex::LookupToEdit (const HashIndex::LookupRequest &_request) noexcept
{
    using IteratorPair = std::pair<RecordHashSetIterator, RecordHashSetIterator>;
    auto [begin, end] = EMERGENCE_UNION2_CALL (records, implementationSwitch, IteratorPair, equal_range, _request);
    return {this, begin, end};
}
} // namespace Emergence::Pegasus
