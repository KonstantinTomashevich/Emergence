#include <cassert>

#include <Hashing/ByteHasher.hpp>

#include <Pegasus/HashIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
const InplaceVector <StandardLayout::Field, HashIndex::MAX_INDEXED_FIELDS> &HashIndex::GetIndexedFields () const noexcept
{
    return indexedFields;
}

bool HashIndex::CanBeDropped () const noexcept
{
    // TODO: Cursors must point to index through Handling too.
    return GetReferenceCount () == 0u;
}

void HashIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (owner);
    owner->DropIndex (*this);
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

bool HashIndex::Comparator::operator () (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    for (const StandardLayout::Field &indexedField : owner->GetIndexedFields ())
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreFieldValuesEqual (_firstRecord, _secondRecord, indexedField))
        {
            return false;
        }
    }

    return true;
}

bool HashIndex::Comparator::operator () (const void *_firstRecord,
                                         const HashIndex::RecordWithBackup &_recordWithBackup) const noexcept
{
    return _firstRecord == _recordWithBackup.record;
}

HashIndex::HashIndex (Emergence::Pegasus::Storage *_owner, std::size_t _initialBuckets,
                      const std::vector <StandardLayout::Field> &_indexedFields)
    : Handling::HandleableBase (),
      owner (_owner),
      records (_initialBuckets, Hasher {this}, Comparator {this})
{
    // Add self reference to prevent Handling from deleting this object.
    // Storage owns indices and Handling is used only to check if dropping index is safe.
    RegisterReference ();

    assert (_owner);
    assert (!_indexedFields.empty ());
    assert (_indexedFields.size () < MAX_INDEXED_FIELDS);

    std::size_t indexedFieldsCount = std::min (MAX_INDEXED_FIELDS, _indexedFields.size ());
    for (std::size_t index = 0u; index < indexedFieldsCount; ++index)
    {
        indexedFields.EmplaceBack (_indexedFields[index]);
    }
}

void HashIndex::InsertRecord (const void *_record) noexcept
{
    records.emplace (_record);
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
} // namespace Emergence::Pegasus