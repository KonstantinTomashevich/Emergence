#include <Assert/Assert.hpp>

#include <StandardLayout/Original/PlainPatch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
Patch PatchBuilder::FromDifference (const Mapping &_typeMapping, const void *_changed, const void *_initial) noexcept
{
    EMERGENCE_ASSERT (_changed);
    EMERGENCE_ASSERT (_initial);
    PlainPatchBuilder builder;
    builder.Begin (_typeMapping);

    for (auto iterator = _typeMapping.BeginConditional (_changed), end = _typeMapping.EndConditional ();
         iterator != end; ++iterator)
    {
        Field field = *iterator;
        const uint8_t *changedAddress = static_cast<const uint8_t *> (_changed) + field.GetOffset ();
        const uint8_t *initialAddress = static_cast<const uint8_t *> (_initial) + field.GetOffset ();

        switch (field.GetArchetype ())
        {
        case FieldArchetype::BIT:
        {
            const uint8_t mask = 1u << field.GetBitOffset ();
            if ((*changedAddress & mask) != (*initialAddress & mask))
            {
                builder.Set (_typeMapping.GetFieldId (field),
                             array_cast<bool, VALUE_MAX_SIZE> (*changedAddress & mask));
            }

            break;
        }

#define GET(Type, address) *reinterpret_cast<const Type *> (address)
#define DO_CHECK(Type)                                                                                                 \
    if (GET (Type, changedAddress) != GET (Type, initialAddress))                                                      \
    {                                                                                                                  \
        builder.Set (_typeMapping.GetFieldId (field), array_cast<Type, VALUE_MAX_SIZE> (GET (Type, changedAddress)));  \
    }

        case FieldArchetype::INT:
            switch (field.GetSize ())
            {
            case 1u:
                DO_CHECK (int8_t)
                break;

            case 2u:
                DO_CHECK (int16_t)
                break;

            case 4u:
                DO_CHECK (int32_t)
                break;

            case 8u:
                DO_CHECK (int64_t)
                break;
            }
            break;

        case FieldArchetype::UINT:
            switch (field.GetSize ())
            {
            case 1u:
                DO_CHECK (uint8_t)
                break;

            case 2u:
                DO_CHECK (uint16_t)
                break;

            case 4u:
                DO_CHECK (uint32_t)
                break;

            case 8u:
                DO_CHECK (uint64_t)
                break;
            }
            break;

        case FieldArchetype::FLOAT:
            switch (field.GetSize ())
            {
            case 4u:
                DO_CHECK (float)
                break;

            case 8u:
                DO_CHECK (double)
                break;
            }
            break;

        case FieldArchetype::UNIQUE_STRING:
            DO_CHECK (Memory::UniqueString)
            break;

#undef DO_CHECK
#undef GET

        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
        case FieldArchetype::NESTED_OBJECT:
            // Do nothing: unsupported archetypes. Nested field is actually supported
            // due to projection, but its whole-field registration is ignored like that.
            break;
        }
    }

    Handling::Handle<PlainPatch> patch = builder.End ();
    return Patch (array_cast (patch));
}

PatchBuilder::PatchBuilder () noexcept
{
    new (&data) PlainPatchBuilder ();
}

PatchBuilder::PatchBuilder (PatchBuilder &&_other) noexcept
{
    new (&data) PlainPatchBuilder (std::move (block_cast<PlainPatchBuilder> (_other.data)));
}

PatchBuilder::~PatchBuilder () noexcept
{
    block_cast<PlainPatchBuilder> (data).~PlainPatchBuilder ();
}

void PatchBuilder::Begin (const Mapping &_typeMapping) noexcept
{
    block_cast<PlainPatchBuilder> (data).Begin (_typeMapping);
}

Patch PatchBuilder::End () noexcept
{
    Handling::Handle<PlainPatch> patch = block_cast<PlainPatchBuilder> (data).End ();
    return Patch (array_cast (patch));
}

void PatchBuilder::SetBit (FieldId _field, bool _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<bool, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetInt8 (FieldId _field, int8_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<int8_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetInt16 (FieldId _field, int16_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<int16_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetInt32 (FieldId _field, int32_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<int32_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetInt64 (FieldId _field, int64_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<int64_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetUInt8 (FieldId _field, uint8_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<uint8_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetUInt16 (FieldId _field, uint16_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<uint16_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetUInt32 (FieldId _field, uint32_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<uint32_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetUInt64 (FieldId _field, uint64_t _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<uint64_t, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetFloat (FieldId _field, float _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<float, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetDouble (FieldId _field, double _value) noexcept
{
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<double, VALUE_MAX_SIZE> (_value));
}

void PatchBuilder::SetUniqueString (FieldId _field, const Memory::UniqueString &_value) noexcept
{
    static_assert (std::is_trivially_move_constructible_v<Memory::UniqueString>);
    static_assert (std::is_trivially_destructible_v<Memory::UniqueString>);
    block_cast<PlainPatchBuilder> (data).Set (_field, array_cast<Memory::UniqueString, VALUE_MAX_SIZE> (_value));
}
} // namespace Emergence::StandardLayout
