#include <cassert>

#include <Hashing/ByteHasher.hpp>

#include <Pegasus/HashIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
const InplaceVector <StandardLayout::Field, Constants::HashIndex::MAX_INDEXED_FIELDS> &
HashIndex::GetIndexedFields () const noexcept
{
    return indexedFields;
}

bool HashIndex::CanBeDropped () const noexcept
{
    // Self reference is always here.
    return GetReferenceCount () == 1u && activeCursors == 0u;
}

void HashIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

std::size_t HashIndex::Hasher::operator () (const void *_record) const noexcept
{
    assert (owner);
    assert (_record);

    Hashing::ByteHasher hasher;
    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                hasher.Append (
                    static_cast <const uint8_t *> (indexedField.GetValue (_record)),
                    indexedField.GetSize ());
                break;
            }

            case StandardLayout::FieldArchetype::BIT:
            {
                // Currently we hash bits as bytes that could have 1 only on given bit offset.
                uint8_t mask = 1u << indexedField.GetBitOffset ();

                if (*static_cast <const uint8_t *> (indexedField.GetValue (_record)) & mask)
                {
                    hasher.Append (mask);
                }
                else
                {
                    hasher.Append (0u);
                }

                break;
            }

            case StandardLayout::FieldArchetype::STRING:
                const auto *current = static_cast <const uint8_t *> (indexedField.GetValue (_record));
                const uint8_t *stringEnd = current + indexedField.GetSize ();

                while (*current && current != stringEnd)
                {
                    hasher.Append (*current);
                    ++current;
                }

                break;
        }
    }

    return hasher.GetCurrentValue () % std::numeric_limits <std::size_t>::max ();
}

std::size_t HashIndex::Hasher::operator () (const HashIndex::RecordWithBackup &_record) const noexcept
{
    return (*this) (_record.backup);
}

std::size_t HashIndex::Hasher::operator () (const HashIndex::LookupRequest &_request) const noexcept
{
    assert (owner);
    assert (_request.indexedFieldValues);

    Hashing::ByteHasher hasher;
    const auto *currentFieldBegin = static_cast <const uint8_t *> (_request.indexedFieldValues);

    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                hasher.Append (currentFieldBegin, indexedField.GetSize ());
                break;
            }

            case StandardLayout::FieldArchetype::BIT:
            {
                // Currently we hash bits as bytes that could have 1 only on given bit offset.
                uint8_t mask = 1u << indexedField.GetBitOffset ();

                if (*currentFieldBegin & mask)
                {
                    hasher.Append (mask);
                }
                else
                {
                    hasher.Append (0u);
                }

                break;
            }

            case StandardLayout::FieldArchetype::STRING:
                const auto *current = currentFieldBegin;
                const uint8_t *stringEnd = current + indexedField.GetSize ();

                while (*current && current != stringEnd)
                {
                    hasher.Append (*current);
                    ++current;
                }

                break;
        }

        currentFieldBegin += indexedField.GetSize ();
    }

    return hasher.GetCurrentValue () % std::numeric_limits <std::size_t>::max ();
}

bool HashIndex::Comparator::operator () (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    assert (_firstRecord);
    assert (_secondRecord);

    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreRecordValuesEqual (_firstRecord, _secondRecord, indexedField))
        {
            return false;
        }
    }

    return true;
}

bool HashIndex::Comparator::operator () (const void *_record,
                                         const HashIndex::RecordWithBackup &_recordWithBackup) const noexcept
{
    return _record == _recordWithBackup.record;
}

bool HashIndex::Comparator::operator () (const void *_record, const HashIndex::LookupRequest &_request) const noexcept
{
    assert (_record);
    assert (_request.indexedFieldValues);

    const auto *currentFieldBegin = static_cast <const uint8_t *> (_request.indexedFieldValues);

    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreFieldValuesEqual (indexedField.GetValue (_record), currentFieldBegin, indexedField))
        {
            return false;
        }

        currentFieldBegin += indexedField.GetSize ();
    }

    return true;
}

bool HashIndex::Comparator::operator () (const HashIndex::LookupRequest &_request, const void *_record) const noexcept
{
    return (*this) (_record, _request);
}

HashIndex::HashIndex (Emergence::Pegasus::Storage *_owner, std::size_t _initialBuckets,
                      const std::vector <StandardLayout::FieldId> &_indexedFields)
    : Handling::HandleableBase (),
      storage (_owner),
      records (_initialBuckets, Hasher {this}, Comparator {this})
{
    // Add self reference to prevent Handling from deleting this object.
    // Storage owns indices and Handling is used only to check if dropping index is safe.
    RegisterReference ();

    assert (storage);
    assert (!_indexedFields.empty ());
    assert (_indexedFields.size () < Constants::HashIndex::MAX_INDEXED_FIELDS);

    std::size_t indexedFieldsCount = std::min (Constants::HashIndex::MAX_INDEXED_FIELDS, _indexedFields.size ());
    const StandardLayout::Mapping &recordMapping = storage->GetRecordMapping ();

    for (std::size_t index = 0u; index < indexedFieldsCount; ++index)
    {
        StandardLayout::Field indexedField = recordMapping.GetField (_indexedFields[index]);
        assert (indexedField.IsHandleValid ());
        indexedFields.EmplaceBack (indexedField);
    }
}

void HashIndex::InsertRecord (const void *_record) noexcept
{
    records.emplace (_record);
}

void HashIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    auto iterator = records.find (RecordWithBackup {_record, _recordBackup});
    assert (iterator != records.end ());
    records.erase (iterator);
}

void HashIndex::DeleteRecordMyself (RecordHashSet::iterator _position) noexcept
{
    assert (_position != records.end ());
    const void *record = *_position;
    records.erase (_position);
    storage->DeleteRecord (record, this);
}

void HashIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    auto iterator = records.find (RecordWithBackup {_record, _recordBackup});
    assert (iterator != records.end ());
    changedNodes.emplace_back (records.extract (iterator));
}

void HashIndex::OnWriterClosed () noexcept
{
    // Reinsert extracted changed nodes back into index.
    for (auto &changedNode : changedNodes)
    {
        assert (changedNode);
        records.insert (std::move (changedNode));
    }

    changedNodes.clear ();
}

HashIndex::ReadCursor::ReadCursor (const HashIndex::ReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

HashIndex::ReadCursor::ReadCursor (HashIndex::ReadCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
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

const void *HashIndex::ReadCursor::operator * () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

HashIndex::ReadCursor &HashIndex::ReadCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

HashIndex::ReadCursor::ReadCursor (HashIndex *_index,
                                   RecordHashSet::const_iterator _begin,
                                   RecordHashSet::const_iterator _end) noexcept
    : index (_index),
      current (_begin),
      end (_end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

HashIndex::EditCursor::EditCursor (HashIndex::EditCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    _other.index = nullptr;
}

HashIndex::EditCursor::~EditCursor () noexcept
{
    if (index)
    {
        EndRecordEdition ();
        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *HashIndex::EditCursor::operator * () noexcept
{
    assert (index);
    return current != end ? const_cast <void *> (*current) : nullptr;
}

HashIndex::EditCursor &HashIndex::EditCursor::operator ~ ()
{
    assert (index);
    assert (current != end);
    auto next = std::next (current);

    index->DeleteRecordMyself (current);
    current = next;
    return *this;
}

HashIndex::EditCursor &HashIndex::EditCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);
    auto next = std::next (current);
    EndRecordEdition ();

    current = next;
    BeginRecordEdition ();
    return *this;
}

HashIndex::EditCursor::EditCursor (HashIndex *_index,
                                   RecordHashSet::iterator _begin,
                                   RecordHashSet::iterator _end) noexcept
    : index (_index),
      current (_begin),
      end (_end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void HashIndex::EditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

void HashIndex::EditCursor::EndRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->EndRecordEdition (*current);
    }
}

HashIndex::ReadCursor HashIndex::LookupToRead (const HashIndex::LookupRequest &_request) noexcept
{
    auto[begin, end] = records.equal_range (_request);
    return HashIndex::ReadCursor (this, begin, end);
}

HashIndex::EditCursor HashIndex::LookupToEdit (const HashIndex::LookupRequest &_request) noexcept
{
    auto[begin, end] = records.equal_range (_request);
    return HashIndex::EditCursor (this, begin, end);
}
} // namespace Emergence::Pegasus