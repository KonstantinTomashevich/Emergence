#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
FieldId ProjectNestedField (FieldId objectField, FieldId nestedField) noexcept
{
    return objectField + nestedField + 1u;
}

void *FieldMeta::GetValue (void *_object) const noexcept
{
    return static_cast <uint8_t *> (_object) + offset;
}

const void *FieldMeta::GetValue (const void *_object) const noexcept
{
    return static_cast <const uint8_t *> (_object) + offset;
}
} // namespace Emergence::StandardLayout