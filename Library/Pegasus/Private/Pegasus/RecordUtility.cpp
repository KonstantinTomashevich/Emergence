#include <cassert>
#include <cstring>

#include <Pegasus/RecordUtility.hpp>

namespace Emergence::Pegasus
{
bool AreFieldValuesEqual (const void *_firstRecord, const void *_secondRecord,
                          const StandardLayout::Field &_field) noexcept
{
    // _field should be leaf-field, not intermediate nested object.
    assert (_field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::BIT:
        {
            uint8_t mask = 1u << _field.GetBitOffset ();
            if ((*static_cast <const uint8_t *> (_field.GetValue (_firstRecord)) & mask) !=
                (*static_cast <const uint8_t *> (_field.GetValue (_secondRecord)) & mask))
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::INT:
        case StandardLayout::FieldArchetype::UINT:
        case StandardLayout::FieldArchetype::FLOAT:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        {
            if (memcmp (_field.GetValue (_firstRecord), _field.GetValue (_secondRecord), _field.GetSize ()) != 0u)
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::STRING:
        {
            if (strncmp (static_cast <const char *> (_field.GetValue (_firstRecord)),
                         static_cast <const char *> (_field.GetValue (_secondRecord)),
                         _field.GetSize () / sizeof (char)) != 0u)
            {
                return false;
            }

            break;
        }
    }

    return true;
}
} // namespace Emergence::Pegasus