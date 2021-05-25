#define _CRT_SECURE_NO_WARNINGS
#include <cassert>

#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
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