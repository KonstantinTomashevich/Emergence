#include <cassert>

#include <Container/StringBuilder.hpp>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
MappingBuilder::MappingBuilder () noexcept
{
    new (&data) PlainMappingBuilder ();
}

MappingBuilder::MappingBuilder (MappingBuilder &&_other) noexcept
{
    new (&data) PlainMappingBuilder (std::move (block_cast<PlainMappingBuilder> (_other.data)));
}

MappingBuilder::~MappingBuilder () noexcept
{
    block_cast<PlainMappingBuilder> (data).~PlainMappingBuilder ();
}

void MappingBuilder::Begin (Memory::UniqueString _name, std::size_t _objectSize) noexcept
{
    block_cast<PlainMappingBuilder> (data).Begin (_name, _objectSize);
}

Mapping MappingBuilder::End () noexcept
{
    Handling::Handle<PlainMapping> mapping = block_cast<PlainMappingBuilder> (data).End ();
    return Mapping (reinterpret_cast<decltype (Mapping::data) *> (&mapping));
}

void MappingBuilder::SetConstructor (void (*_constructor) (void *)) noexcept
{
    block_cast<PlainMappingBuilder> (data).SetConstructor (_constructor);
}

void MappingBuilder::SetDestructor (void (*_destructor) (void *)) noexcept
{
    block_cast<PlainMappingBuilder> (data).SetDestructor (_destructor);
}

FieldId MappingBuilder::RegisterBit (Memory::UniqueString _name, std::size_t _offset, uint_fast8_t _bitOffset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (FieldData::BitSeed {_name, _offset, _bitOffset});
}

FieldId MappingBuilder::RegisterInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, _offset, sizeof (int8_t)});
}

FieldId MappingBuilder::RegisterInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, _offset, sizeof (int16_t)});
}

FieldId MappingBuilder::RegisterInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, _offset, sizeof (int32_t)});
}

FieldId MappingBuilder::RegisterInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, _offset, sizeof (int64_t)});
}

FieldId MappingBuilder::RegisterUInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, _offset, sizeof (uint8_t)});
}

FieldId MappingBuilder::RegisterUInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, _offset, sizeof (uint16_t)});
}

FieldId MappingBuilder::RegisterUInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, _offset, sizeof (uint32_t)});
}

FieldId MappingBuilder::RegisterUInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, _offset, sizeof (uint64_t)});
}

FieldId MappingBuilder::RegisterFloat (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::FLOAT, _offset, sizeof (float)});
}

FieldId MappingBuilder::RegisterDouble (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::FLOAT, _offset, sizeof (double)});
}

FieldId MappingBuilder::RegisterString (Memory::UniqueString _name, std::size_t _offset, std::size_t _maxSize) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::STRING, _offset, _maxSize});
}

FieldId MappingBuilder::RegisterBlock (Memory::UniqueString _name, std::size_t _offset, std::size_t _size) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::BLOCK, _offset, _size});
}

FieldId MappingBuilder::RegisterUniqueString (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (FieldData::UniqueStringSeed {_name, _offset});
}

FieldId MappingBuilder::RegisterNestedObject (Memory::UniqueString _name,
                                              std::size_t _offset,
                                              const Mapping &_objectMapping) noexcept
{
    auto &state = block_cast<PlainMappingBuilder> (data);

    const auto &nestedPlainMapping = block_cast<Handling::Handle<PlainMapping>> (_objectMapping.data);
    assert (nestedPlainMapping);
    FieldId objectFieldId = state.AddField (FieldData::NestedObjectSeed {_name, _offset, nestedPlainMapping.Get ()});

    for (const FieldData &field : *nestedPlainMapping.Get ())
    {
        const char *fullName = EMERGENCE_BUILD_STRING (_name, PROJECTION_NAME_SEPARATOR, field.GetName ());
        [[maybe_unused]] FieldId nestedFieldId = 0u;

        switch (field.GetArchetype ())
        {
        case FieldArchetype::BIT:
            nestedFieldId = state.AddField (FieldData::BitSeed {Memory::UniqueString {fullName},
                                                                _offset + field.GetOffset (), field.GetBitOffset ()});
            break;

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
            nestedFieldId =
                state.AddField (FieldData::StandardSeed {Memory::UniqueString {fullName}, field.GetArchetype (),
                                                         _offset + field.GetOffset (), field.GetSize ()});
            break;

        case FieldArchetype::UNIQUE_STRING:
            nestedFieldId = state.AddField (
                FieldData::UniqueStringSeed {Memory::UniqueString {fullName}, _offset + field.GetOffset ()});
            break;

        case FieldArchetype::NESTED_OBJECT:
            // We don't need to recursively add fields, because given nested mapping is finished,
            // therefore all fields of internal objects are already projected into this mapping.
            nestedFieldId = state.AddField (FieldData::NestedObjectSeed {
                Memory::UniqueString {fullName}, _offset + field.GetOffset (), field.GetNestedObjectMapping ().Get ()});
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
