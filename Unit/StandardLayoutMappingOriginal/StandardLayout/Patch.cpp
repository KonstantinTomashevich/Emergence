#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Iterator.hpp>

#include <Assert/Assert.hpp>

#include <StandardLayout/Original/PlainPatch.hpp>
#include <StandardLayout/Patch.hpp>

namespace Emergence::StandardLayout
{
using Iterator = Patch::Iterator;

using IteratorImplementation = const ValueSetter *;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (Iterator, IteratorImplementation)

Patch::ChangeInfo Patch::Iterator::operator* () const noexcept
{
    const auto *valueSetter = block_cast<const ValueSetter *> (data);
    return {valueSetter->field, &valueSetter->value};
}

Patch::Patch () noexcept
{
    new (&data) Handling::Handle<PlainPatch> (nullptr);
}

Patch::Patch (const Patch &_other) noexcept
    : Patch (_other.data)
{
}

Patch::Patch (Patch &&_other) noexcept
    : Patch (_other.data)
{
}

Patch::~Patch () noexcept
{
    block_cast<Handling::Handle<PlainPatch>> (data).~Handle ();
}

Mapping Patch::GetTypeMapping () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetTypeMapping ();
}

void Patch::Apply (void *_object) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    EMERGENCE_ASSERT (handle);
    handle->Apply (_object);
}

std::size_t Patch::GetChangeCount () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    EMERGENCE_ASSERT (handle);
    return handle->GetValueCount ();
}

Iterator Patch::Begin () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    EMERGENCE_ASSERT (handle);
    const ValueSetter *iterator = handle->Begin ();
    return Iterator (array_cast (iterator));
}

Iterator Patch::End () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    EMERGENCE_ASSERT (handle);
    const ValueSetter *iterator = handle->End ();
    return Iterator (array_cast (iterator));
}

bool Patch::IsHandleEqual (const Patch &_other) const noexcept
{
    return block_cast<Handling::Handle<PlainPatch>> (data) == block_cast<Handling::Handle<PlainPatch>> (_other.data);
}

Patch Patch::operator+ (const Patch &_other) const noexcept
{
    EMERGENCE_ASSERT (GetTypeMapping () == _other.GetTypeMapping ());
    PlainPatchBuilder builder;
    builder.Begin (GetTypeMapping ());

    for (const Patch::ChangeInfo &info : *this)
    {
        bool overrideFound = false;

        // Not the most performance-friendly logic, but patches are usually small, so this is good enough.
        for (const Patch::ChangeInfo &overriderInfo : _other)
        {
            if (overriderInfo.field == info.field)
            {
                overrideFound = true;
                break;
            }
        }

        if (!overrideFound)
        {
            builder.Set (info.field, *static_cast<const std::array<std::uint8_t, VALUE_MAX_SIZE> *> (info.newValue));
        }
    }

    for (const Patch::ChangeInfo &info : _other)
    {
        builder.Set (info.field, *static_cast<const std::array<std::uint8_t, VALUE_MAX_SIZE> *> (info.newValue));
    }

    Handling::Handle<PlainPatch> patch = builder.End ();
    return Patch (array_cast (patch));
}

Patch Patch::operator- (const Patch &_other) const noexcept
{
    EMERGENCE_ASSERT (GetTypeMapping () == _other.GetTypeMapping ());
    PlainPatchBuilder builder;
    builder.Begin (GetTypeMapping ());

    for (const Patch::ChangeInfo &info : *this)
    {
        bool found = false;

        // Not the most performance-friendly logic, but patches are usually small, so this is good enough.
        for (const Patch::ChangeInfo &overriderInfo : _other)
        {
            if (overriderInfo.field == info.field)
            {
                Field field = GetTypeMapping ().GetField (overriderInfo.field);
                switch (field.GetArchetype ())
                {
#define CHECK(Type)                                                                                                    \
    found = *static_cast<const Type *> (overriderInfo.newValue) == *static_cast<const Type *> (info.newValue);         \
    break
                case FieldArchetype::BIT:
                    CHECK (bool);

                case FieldArchetype::INT:
                    switch (field.GetSize ())
                    {
                    case 1u:
                        CHECK (int8_t);
                    case 2u:
                        CHECK (int16_t);
                    case 4u:
                        CHECK (int32_t);
                    case 8u:
                        CHECK (int64_t);
                    }
                    break;

                case FieldArchetype::UINT:
                    switch (field.GetSize ())
                    {
                    case 1u:
                        CHECK (std::uint8_t);
                    case 2u:
                        CHECK (std::uint16_t);
                    case 4u:
                        CHECK (std::uint32_t);
                    case 8u:
                        CHECK (std::uint64_t);
                    }
                    break;

                case FieldArchetype::FLOAT:
                    switch (field.GetSize ())
                    {
                    case 4u:
                        CHECK (float);
                    case 8u:
                        CHECK (double);
                    }
                    break;

                case FieldArchetype::UNIQUE_STRING:
                    CHECK (Memory::UniqueString);
#undef CHECK

                case FieldArchetype::STRING:
                case FieldArchetype::BLOCK:
                case FieldArchetype::NESTED_OBJECT:
                case FieldArchetype::UTF8_STRING:
                case FieldArchetype::VECTOR:
                case FieldArchetype::PATCH:
                    // Do nothing: unsupported archetypes. Nested field is actually supported
                    // due to projection, but its whole-field registration is ignored like that.
                    break;
                }

                if (found)
                {
                    break;
                }
            }
        }

        if (!found)
        {
            builder.Set (info.field, *static_cast<const std::array<std::uint8_t, VALUE_MAX_SIZE> *> (info.newValue));
        }
    }

    Handling::Handle<PlainPatch> patch = builder.End ();
    return Patch (array_cast (patch));
}

bool Patch::IsHandleValid () const noexcept
{
    return block_cast<Handling::Handle<PlainPatch>> (data).Get ();
}

Patch::operator bool () const noexcept
{
    return IsHandleValid ();
}

Patch &Patch::operator= (const Patch &_other) noexcept
{
    if (this != &_other)
    {
        this->~Patch ();
        new (this) Patch (_other);
    }

    return *this;
}

Patch &Patch::operator= (Patch &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Patch ();
        new (this) Patch (std::move (_other));
    }

    return *this;
}

Patch::Patch (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (block_cast<Handling::Handle<PlainPatch>> (_data));
}

Patch::Patch (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (std::move (block_cast<Handling::Handle<PlainPatch>> (_data)));
}

Patch::Iterator begin (const Patch &_patch) noexcept
{
    return _patch.Begin ();
}

Patch::Iterator end (const Patch &_patch) noexcept
{
    return _patch.End ();
}
} // namespace Emergence::StandardLayout
