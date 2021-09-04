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

Field::Field () noexcept : handle (nullptr)
{
}

Field::Field (const Field &_other) noexcept : Field (_other.handle)
{
}

Field::Field (Field &&_other) noexcept : Field (_other.handle)
{
    _other.handle = nullptr;
}

Field::~Field () noexcept
{
}

FieldArchetype Field::GetArchetype () const noexcept
{
    assert (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetArchetype ();
}

std::size_t Field::GetOffset () const noexcept
{
    assert (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetOffset ();
}

std::size_t Field::GetSize () const noexcept
{
    assert (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetSize ();
}

std::size_t Field::GetBitOffset () const noexcept
{
    assert (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetBitOffset ();
}

Mapping Field::GetNestedObjectMapping () const noexcept
{
    assert (IsHandleValid ());
    Handling::Handle<PlainMapping> nestedMapping = static_cast<const FieldData *> (handle)->GetNestedObjectMapping ();
    return Mapping (reinterpret_cast<decltype (Mapping::data) *> (&nestedMapping));
}

const char *Field::GetName () const noexcept
{
    return static_cast<const FieldData *> (handle)->GetName ();
}

void *Field::GetValue (void *_object) const noexcept
{
    return const_cast<void *> (GetValue (const_cast<const void *> (_object)));
}

const void *Field::GetValue (const void *_object) const noexcept
{
    assert (_object);
    return static_cast<const uint8_t *> (_object) + GetOffset ();
}

bool Field::IsSame (const Field &_other) const noexcept
{
    assert (IsHandleValid ());
    return handle == _other.handle;
}

bool Field::IsHandleValid () const noexcept
{
    return handle;
}

Field::operator bool () const noexcept
{
    return IsHandleValid ();
}

Field &Field::operator= (const Field &_other) noexcept
{
    if (this != &_other)
    {
        this->~Field ();
        new (this) Field (_other);
    }

    return *this;
}

Field &Field::operator= (Field &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Field ();
        new (this) Field (std::move (_other));
    }

    return *this;
}

Field::Field (void *_handle) noexcept : handle (_handle)
{
}
} // namespace Emergence::StandardLayout
