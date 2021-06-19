#include <cassert>
#include <cstring>

#include <Pegasus/RecordUtility.hpp>

namespace Emergence::Pegasus
{
bool AreRecordValuesEqual (const void *_firstRecord, const void *_secondRecord,
                           const StandardLayout::Field &_field) noexcept
{
    return AreFieldValuesEqual (_field.GetValue (_firstRecord), _field.GetValue (_secondRecord), _field);
}

bool AreFieldValuesEqual (const void *_firstRecordValue, const void *_secondRecordValue,
                          const StandardLayout::Field &_field) noexcept
{
    // _field should be leaf-field, not intermediate nested object.
    assert (_field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::BIT:
        {
            uint8_t mask = 1u << _field.GetBitOffset ();
            if ((*static_cast <const uint8_t *> (_firstRecordValue) & mask) !=
                (*static_cast <const uint8_t *> (_secondRecordValue) & mask))
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
            if (memcmp (_firstRecordValue, _secondRecordValue, _field.GetSize ()) != 0u)
            {
                return false;
            }

            break;
        }

        case StandardLayout::FieldArchetype::STRING:
        {
            if (strncmp (static_cast <const char *> (_firstRecordValue),
                         static_cast <const char *> (_secondRecordValue),
                         _field.GetSize () / sizeof (char)) != 0u)
            {
                return false;
            }

            break;
        }
    }

    return true;
}

bool IsFieldValueLesser (const void *_firstValue, const void *_secondValue,
                         const StandardLayout::Field &_field) noexcept
{
    // _field should be leaf-field, not intermediate nested object.
    assert (_field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::BIT:
        {
            uint8_t mask = 1u << _field.GetBitOffset ();
            return (*static_cast <const uint8_t *> (_firstValue) & mask) <
                   (*static_cast <const uint8_t *> (_secondValue) & mask);
        }

        case StandardLayout::FieldArchetype::INT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (int8_t):
                    return *static_cast <const int8_t *> (_firstValue) <
                           *static_cast <const int8_t *> (_secondValue);

                case sizeof (int16_t):
                    return *static_cast <const int16_t *> (_firstValue) <
                           *static_cast <const int16_t *> (_secondValue);

                case sizeof (int32_t):
                    return *static_cast <const int32_t *> (_firstValue) <
                           *static_cast <const int32_t *> (_secondValue);

                case sizeof (int64_t):
                    return *static_cast <const int64_t *> (_firstValue) <
                           *static_cast <const int64_t *> (_secondValue);

                default:
                {
                    assert (false);
                }
            }

            break;
        }
        case StandardLayout::FieldArchetype::UINT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (uint8_t):
                    return *static_cast <const uint8_t *> (_firstValue) <
                           *static_cast <const uint8_t *> (_secondValue);

                case sizeof (uint16_t):
                    return *static_cast <const uint16_t *> (_firstValue) <
                           *static_cast <const uint16_t *> (_secondValue);

                case sizeof (uint32_t):
                    return *static_cast <const uint32_t *> (_firstValue) <
                           *static_cast <const uint32_t *> (_secondValue);

                case sizeof (uint64_t):
                    return *static_cast <const uint64_t *> (_firstValue) <
                           *static_cast <const uint64_t *> (_secondValue);

                default:
                {
                    assert (false);
                }
            }

            break;
        }

        case StandardLayout::FieldArchetype::FLOAT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (float):
                    return *static_cast <const float *> (_firstValue) <
                           *static_cast <const float *> (_secondValue);

                case sizeof (double):
                    return *static_cast <const double *> (_firstValue) <
                           *static_cast <const double *> (_secondValue);

                default:
                {
                    assert (false);
                }
            }

            break;
        }
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        {
            return memcmp (_firstValue, _secondValue, _field.GetSize ()) < 0u;
        }

        case StandardLayout::FieldArchetype::STRING:
        {
            return strncmp (static_cast <const char *> (_firstValue),
                            static_cast <const char *> (_secondValue),
                            _field.GetSize () / sizeof (char)) < 0u;
        }
    }

    return true;
}
} // namespace Emergence::Pegasus