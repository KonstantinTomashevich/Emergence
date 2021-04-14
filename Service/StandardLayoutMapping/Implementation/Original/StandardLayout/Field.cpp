#include <cassert>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
FieldId ProjectNestedField (FieldId objectField, FieldId nestedField) noexcept
{
    return objectField + nestedField + 1u;
}

FieldArchetype Field::GetArchetype () const noexcept
{
    assert (IsHandleValid ());
    return static_cast <const FieldData *> (handle)->GetArchetype ();
}

std::size_t Field::GetOffset () const noexcept
{
    assert (IsHandleValid ());
    return static_cast <const FieldData *> (handle)->GetOffset ();
}

std::size_t Field::GetSize () const noexcept
{
    assert (IsHandleValid ());
    return static_cast <const FieldData *> (handle)->GetSize ();
}

std::size_t Field::GetBitOffset () const noexcept
{
    assert (IsHandleValid ());
    return static_cast <const FieldData *> (handle)->GetBitOffset ();
}

Mapping Field::GetInstanceMapping () const noexcept
{
    assert (IsHandleValid ());
    return Mapping (static_cast <const FieldData *> (handle)->GetInstanceMapping ());
}

void *Field::GetValue (void *_object) const noexcept
{
    return const_cast <void *> (GetValue (const_cast <const void *> (_object)));
}

const void *Field::GetValue (const void *_object) const noexcept
{
    return static_cast <const uint8_t *> (_object) + GetOffset ();
}

bool Field::IsHandleValid () const
{
    return handle;
}

Field::operator bool () const
{
    return IsHandleValid ();
}

Field::Field (void *_handle)
    : handle (_handle)
{
}

Field::~Field ()
{
}
} // namespace Emergence::StandardLayout