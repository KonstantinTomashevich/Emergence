#include <Assert/Assert.hpp>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence
{
namespace StandardLayout
{
FieldId ProjectNestedField (FieldId _objectField, FieldId _nestedField) noexcept
{
    return _objectField + _nestedField + 1u;
}

Field::Field () noexcept
    : handle (nullptr)
{
}

Field::Field (const Field &_other) noexcept
    : Field (_other.handle)
{
}

Field::Field (Field &&_other) noexcept
    : Field (_other.handle)
{
    _other.handle = nullptr;
}

Field::~Field () noexcept = default;

FieldArchetype Field::GetArchetype () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetArchetype ();
}

bool Field::IsProjected () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->IsProjected ();
}

std::size_t Field::GetOffset () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetOffset ();
}

std::size_t Field::GetSize () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetSize ();
}

std::size_t Field::GetBitOffset () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    return static_cast<const FieldData *> (handle)->GetBitOffset ();
}

Mapping Field::GetNestedObjectMapping () const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
    Handling::Handle<PlainMapping> nestedMapping = static_cast<const FieldData *> (handle)->GetNestedObjectMapping ();
    return Mapping (array_cast (nestedMapping));
}

Memory::UniqueString Field::GetName () const noexcept
{
    return static_cast<const FieldData *> (handle)->GetName ();
}

void *Field::GetValue (void *_object) const noexcept
{
    return const_cast<void *> (GetValue (const_cast<const void *> (_object)));
}

const void *Field::GetValue (const void *_object) const noexcept
{
    EMERGENCE_ASSERT (_object);
    return static_cast<const uint8_t *> (_object) + GetOffset ();
}

bool Field::IsSame (const Field &_other) const noexcept
{
    EMERGENCE_ASSERT (IsHandleValid ());
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

Field::Field (void *_handle) noexcept
    : handle (_handle)
{
}
} // namespace StandardLayout

namespace Memory
{
using namespace Literals;

Profiler::AllocationGroup DefaultAllocationGroup<StandardLayout::FieldId>::Get () noexcept
{
    return Profiler::AllocationGroup {"FieldReflection"_us};
}
} // namespace Memory
} // namespace Emergence
