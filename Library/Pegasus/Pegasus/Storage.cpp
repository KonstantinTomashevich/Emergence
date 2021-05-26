#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
Storage::Allocator::Allocator (Storage::Allocator &&_other) noexcept
    : owner (_other.owner),
      current (_other.current)
{
    _other.owner = nullptr;
    _other.current = nullptr;
}

Storage::Allocator::~Allocator () noexcept
{
    if (owner && current)
    {
        owner->InsertRecord (current);
    }
}

void *Storage::Allocator::Next () noexcept
{
    assert (owner);
    if (current)
    {
        owner->InsertRecord (current);
    }

    current = owner->AllocateRecord ();
    return current;
}

Storage::Allocator::Allocator (Storage *_owner)
    : owner (_owner)
{
    assert (owner);
    owner->RegisterWriter ();
}

Storage::Storage (StandardLayout::Mapping _recordMapping) noexcept
    : records (_recordMapping.GetObjectSize ()),
      reflection {.recordMapping = std::move (_recordMapping)}
{
}

const std::vector <std::unique_ptr <HashIndex>> &Storage::GetHashIndices () noexcept
{
    return indices.hash;
}

const std::vector <std::unique_ptr <OrderedIndex>> &Storage::GetOrderedIndices () noexcept
{
    return indices.ordered;
}

const std::vector <std::unique_ptr <VolumetricIndex>> &Storage::GetVolumetricIndices () noexcept
{
    return indices.volumetric;
}

void Storage::RegisterReader () noexcept
{
    assert (accessCounter.writers == 0u);
    ++accessCounter.readers;
}

void Storage::RegisterWriter () noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);
    ++accessCounter.writers;
}

void *Storage::AllocateRecord () noexcept
{
    return records.Acquire ();
}

void Storage::InsertRecord (const void *record) noexcept
{
    assert (record);
    for (std::unique_ptr <HashIndex> &index : indices.hash)
    {
        // index->InsertRecord (record);
    }

    for (std::unique_ptr <OrderedIndex> &index : indices.ordered)
    {
        // index->InsertRecord (record);
    }

    for (std::unique_ptr <VolumetricIndex> &index : indices.volumetric)
    {
        // index->InsertRecord (record);
    }
}

void Storage::BeginRecordEdition (const void *record) noexcept
{
    assert (record);
    assert (editionState.recordBuffer);

    auto iterator = reflection.indexedFields.begin ();
    auto end = reflection.indexedFields.begin () + reflection.indexedFieldsCount;

    while (iterator != end)
    {
        const IndexedField &indexedField = *iterator;

        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.field.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::BIT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                uint8_t mask = 1u << indexedField.field.GetBitOffset ();

                if (*(static_cast <const uint8_t *> (record) + offset) & mask)
                {
                    *(static_cast <uint8_t *> (editionState.recordBuffer) + offset) |= mask;
                }
                else
                {
                    *(static_cast <uint8_t *> (editionState.recordBuffer) + offset) &= ~mask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                memcpy (static_cast <uint8_t *> (editionState.recordBuffer) + offset,
                        static_cast <const uint8_t *> (record) + offset,
                        indexedField.field.GetSize ());
                break;
            }

            case StandardLayout::FieldArchetype::STRING:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                strncpy (static_cast <char *> (editionState.recordBuffer) + offset,
                         static_cast <const char *> (record) + offset,
                         indexedField.field.GetSize () / sizeof (char));
                break;
            }
        }

        ++iterator;
    }
}

void Storage::EndRecordEdition (const void *record) noexcept
{
    assert (record);
    assert (editionState.recordBuffer);

    ChangedRecord changedRecord {record, 0u};
    decltype (ChangedRecord::changedIndexedFields) fieldMask = 1u;

    auto iterator = reflection.indexedFields.begin ();
    auto end = reflection.indexedFields.begin () + reflection.indexedFieldsCount;

    while (iterator != end)
    {
        const IndexedField &indexedField = *iterator;
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.field.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::BIT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                uint8_t mask = 1u << indexedField.field.GetBitOffset ();

                if ((*(static_cast <const uint8_t *> (record) + offset) & mask) !=
                    (*(static_cast <const uint8_t *> (editionState.recordBuffer) + offset) & mask))
                {
                    changedRecord.changedIndexedFields |= fieldMask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                if (memcmp (static_cast <const uint8_t *> (editionState.recordBuffer) + offset,
                            static_cast <const uint8_t *> (record) + offset,
                            indexedField.field.GetSize ()) != 0u)
                {
                    changedRecord.changedIndexedFields |= fieldMask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::STRING:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                if (strncmp (static_cast <const char *> (editionState.recordBuffer) + offset,
                             static_cast <const char *> (record) + offset,
                             indexedField.field.GetSize () / sizeof (char)) != 0u)
                {
                    changedRecord.changedIndexedFields |= fieldMask;
                }

                break;
            }
        }

        ++iterator;
        fieldMask <<= 1u;
    }

    if (changedRecord.changedIndexedFields != 0u)
    {
        editionState.changedRecords.emplace_back (changedRecord);
    }
}
} // namespace Emergence::Pegasus