#include <cassert>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
MappingBuilder::MappingBuilder () noexcept
{
    handle = new PlainMappingBuilder ();
}

MappingBuilder::MappingBuilder (MappingBuilder &&_other)
    : handle (_other.handle)
{
    _other.handle = nullptr;
}

MappingBuilder::~MappingBuilder () noexcept
{
    if (handle)
    {
        delete static_cast <PlainMappingBuilder *> (handle);
    }
}

void MappingBuilder::Begin (std::size_t _objectSize) noexcept
{
    assert (handle);
    static_cast <PlainMappingBuilder *> (handle)->Begin (_objectSize);
}

Mapping MappingBuilder::End () noexcept
{
    assert (handle);
    Handling::Handle <PlainMapping> mapping = static_cast <PlainMappingBuilder *> (handle)->End ();
    return Mapping (reinterpret_cast <decltype (Mapping::data) *> (&mapping));
}

FieldId MappingBuilder::RegisterBit (std::size_t _offset, uint_fast8_t _bitOffset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField ({_offset, _bitOffset});
}

FieldId MappingBuilder::RegisterInt8 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int8_t)});
}

FieldId MappingBuilder::RegisterInt16 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int16_t)});
}

FieldId MappingBuilder::RegisterInt32 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int32_t)});
}

FieldId MappingBuilder::RegisterInt64 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int64_t)});
}

FieldId MappingBuilder::RegisterUInt8 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint8_t)});
}

FieldId MappingBuilder::RegisterUInt16 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint16_t)});
}

FieldId MappingBuilder::RegisterUInt32 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint32_t)});
}

FieldId MappingBuilder::RegisterUInt64 (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint64_t)});
}

FieldId MappingBuilder::RegisterFloat (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::FLOAT, _offset, sizeof (float)});
}

FieldId MappingBuilder::RegisterDouble (std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::FLOAT, _offset, sizeof (double)});
}

FieldId MappingBuilder::RegisterString (std::size_t _offset, std::size_t _maxSize) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::STRING, _offset, _maxSize});
}

FieldId MappingBuilder::RegisterBlock (std::size_t _offset, std::size_t _size) noexcept
{
    assert (handle);
    return static_cast <PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::BLOCK, _offset, _size});
}

FieldId MappingBuilder::RegisterNestedObject (std::size_t _offset, const Mapping &objectMapping) noexcept
{
    assert (handle);
    auto *state = static_cast <PlainMappingBuilder *> (handle);

    const auto &nestedPlainMapping = block_cast <Handling::Handle <PlainMapping>> (objectMapping.data);
    assert (nestedPlainMapping);
    FieldId objectFieldId = state->AddField ({_offset, nestedPlainMapping.Get ()});

    for (const FieldData &field : *nestedPlainMapping.Get ())
    {
        FieldId nestedFieldId;
        switch (field.GetArchetype ())
        {
            case FieldArchetype::BIT:
                nestedFieldId = state->AddField (
                    {_offset + field.GetOffset (), field.GetBitOffset ()});
                break;

            case FieldArchetype::INT:
            case FieldArchetype::UINT:
            case FieldArchetype::FLOAT:
            case FieldArchetype::STRING:
            case FieldArchetype::BLOCK:
                nestedFieldId = state->AddField (
                    {field.GetArchetype (), _offset + field.GetOffset (), field.GetSize ()});
                break;

            case FieldArchetype::NESTED_OBJECT:
                // We don't need to recursively add fields, because given nested mapping is finished,
                // therefore all fields of internal objects are already projected into this mapping.
                nestedFieldId = state->AddField (
                    {_offset + field.GetOffset (), field.GetNestedObjectMapping ().Get ()});
                break;
        }

        assert (nestedFieldId == ProjectNestedField (objectFieldId, nestedPlainMapping->GetFieldId (field)));
    }

    return objectFieldId;
}

MappingBuilder &MappingBuilder::operator = (MappingBuilder &&_other) noexcept
{
    if (this != &_other)
    {
        this->~MappingBuilder ();
        new (this) MappingBuilder (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::StandardLayout