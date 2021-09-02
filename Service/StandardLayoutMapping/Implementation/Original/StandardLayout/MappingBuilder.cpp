#include <cassert>
#include <string>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
MappingBuilder::MappingBuilder () noexcept
{
    handle = new PlainMappingBuilder ();
}

MappingBuilder::MappingBuilder (MappingBuilder &&_other) noexcept : handle (_other.handle)
{
    _other.handle = nullptr;
}

MappingBuilder::~MappingBuilder () noexcept
{
    if (handle)
    {
        delete static_cast<PlainMappingBuilder *> (handle);
    }
}

void MappingBuilder::Begin (const char *_name, std::size_t _objectSize) noexcept
{
    assert (handle);
    static_cast<PlainMappingBuilder *> (handle)->Begin (_name, _objectSize);
}

Mapping MappingBuilder::End () noexcept
{
    assert (handle);
    Handling::Handle<PlainMapping> mapping = static_cast<PlainMappingBuilder *> (handle)->End ();
    return Mapping (reinterpret_cast<decltype (Mapping::data) *> (&mapping));
}

FieldId MappingBuilder::RegisterBit (const char *_name, std::size_t _offset, uint_fast8_t _bitOffset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField ({_offset, _bitOffset, _name});
}

FieldId MappingBuilder::RegisterInt8 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int8_t), _name});
}

FieldId MappingBuilder::RegisterInt16 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int16_t), _name});
}

FieldId MappingBuilder::RegisterInt32 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int32_t), _name});
}

FieldId MappingBuilder::RegisterInt64 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::INT, _offset, sizeof (int64_t), _name});
}

FieldId MappingBuilder::RegisterUInt8 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint8_t), _name});
}

FieldId MappingBuilder::RegisterUInt16 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint16_t), _name});
}

FieldId MappingBuilder::RegisterUInt32 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint32_t), _name});
}

FieldId MappingBuilder::RegisterUInt64 (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::UINT, _offset, sizeof (uint64_t), _name});
}

FieldId MappingBuilder::RegisterFloat (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::FLOAT, _offset, sizeof (float), _name});
}

FieldId MappingBuilder::RegisterDouble (const char *_name, std::size_t _offset) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField (
        {FieldArchetype::FLOAT, _offset, sizeof (double), _name});
}

FieldId MappingBuilder::RegisterString (const char *_name, std::size_t _offset, std::size_t _maxSize) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField ({FieldArchetype::STRING, _offset, _maxSize, _name});
}

FieldId MappingBuilder::RegisterBlock (const char *_name, std::size_t _offset, std::size_t _size) noexcept
{
    assert (handle);
    return static_cast<PlainMappingBuilder *> (handle)->AddField ({FieldArchetype::BLOCK, _offset, _size, _name});
}

FieldId MappingBuilder::RegisterNestedObject (const char *_name,
                                              std::size_t _offset,
                                              const Mapping &objectMapping) noexcept
{
    assert (handle);
    auto *state = static_cast<PlainMappingBuilder *> (handle);

    const auto &nestedPlainMapping = block_cast<Handling::Handle<PlainMapping>> (objectMapping.data);
    assert (nestedPlainMapping);
    FieldId objectFieldId = state->AddField ({_offset, nestedPlainMapping.Get (), _name});
    const std::string prefix = std::string (_name) + "::";

    for (const FieldData &field : *nestedPlainMapping.Get ())
    {
        const std::string fullName = prefix + field.GetName ();
#ifndef NDEBUG
        FieldId nestedFieldId = 0u;
#endif

        switch (field.GetArchetype ())
        {
        case FieldArchetype::BIT:
#ifndef NDEBUG
            nestedFieldId =
#endif
                state->AddField ({_offset + field.GetOffset (), field.GetBitOffset (), fullName.c_str ()});
            break;

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
#ifndef NDEBUG
            nestedFieldId =
#endif
                state->AddField (
                    {field.GetArchetype (), _offset + field.GetOffset (), field.GetSize (), fullName.c_str ()});
            break;

        case FieldArchetype::NESTED_OBJECT:
            // We don't need to recursively add fields, because given nested mapping is finished,
            // therefore all fields of internal objects are already projected into this mapping.
#ifndef NDEBUG
            nestedFieldId =
#endif
                state->AddField (
                    {_offset + field.GetOffset (), field.GetNestedObjectMapping ().Get (), fullName.c_str ()});
            break;
        }

        assert (nestedFieldId == ProjectNestedField (objectFieldId, nestedPlainMapping->GetFieldId (field)));
    }

    return objectFieldId;
}

MappingBuilder &MappingBuilder::operator= (MappingBuilder &&_other) noexcept
{
    if (this != &_other)
    {
        this->~MappingBuilder ();
        new (this) MappingBuilder (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::StandardLayout
