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

void MappingBuilder::Begin (Memory::UniqueString _name, std::size_t _objectSize, std::size_t _objectAlignment) noexcept
{
    block_cast<PlainMappingBuilder> (data).Begin (_name, _objectSize, _objectAlignment);
}

Mapping MappingBuilder::End () noexcept
{
    Handling::Handle<PlainMapping> mapping = block_cast<PlainMappingBuilder> (data).End ();
    return Mapping (array_cast (mapping));
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
    return block_cast<PlainMappingBuilder> (data).AddField (FieldData::BitSeed {_name, _offset, _bitOffset, false});
}

FieldId MappingBuilder::RegisterInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, false, _offset, sizeof (int8_t)});
}

FieldId MappingBuilder::RegisterInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, false, _offset, sizeof (int16_t)});
}

FieldId MappingBuilder::RegisterInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, false, _offset, sizeof (int32_t)});
}

FieldId MappingBuilder::RegisterInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::INT, false, _offset, sizeof (int64_t)});
}

FieldId MappingBuilder::RegisterUInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, false, _offset, sizeof (uint8_t)});
}

FieldId MappingBuilder::RegisterUInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, false, _offset, sizeof (uint16_t)});
}

FieldId MappingBuilder::RegisterUInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, false, _offset, sizeof (uint32_t)});
}

FieldId MappingBuilder::RegisterUInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::UINT, false, _offset, sizeof (uint64_t)});
}

FieldId MappingBuilder::RegisterFloat (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::FLOAT, false, _offset, sizeof (float)});
}

FieldId MappingBuilder::RegisterDouble (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::FLOAT, false, _offset, sizeof (double)});
}

FieldId MappingBuilder::RegisterString (Memory::UniqueString _name, std::size_t _offset, std::size_t _maxSize) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::STRING, false, _offset, _maxSize});
}

FieldId MappingBuilder::RegisterBlock (Memory::UniqueString _name, std::size_t _offset, std::size_t _size) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (
        FieldData::StandardSeed {_name, FieldArchetype::BLOCK, false, _offset, _size});
}

FieldId MappingBuilder::RegisterUniqueString (Memory::UniqueString _name, std::size_t _offset) noexcept
{
    return block_cast<PlainMappingBuilder> (data).AddField (FieldData::UniqueStringSeed {_name, _offset, false});
}

FieldId MappingBuilder::RegisterNestedObject (Memory::UniqueString _name,
                                              std::size_t _offset,
                                              const Mapping &_objectMapping) noexcept
{
    auto &state = block_cast<PlainMappingBuilder> (data);

    const auto &nestedPlainMapping = block_cast<Handling::Handle<PlainMapping>> (_objectMapping.data);
    assert (nestedPlainMapping);
    FieldId objectFieldId = state.AddField (FieldData::NestedObjectSeed {_name, _offset, nestedPlainMapping.Get ()});

    // We need to simulate conditional iteration in order to copy conditions.
    const ConditionData *topCondition = nullptr;
    const ConditionData *nextCondition = nestedPlainMapping->GetFirstCondition ();

    for (const FieldData &field : *nestedPlainMapping.Get ())
    {
        const FieldId fieldId = nestedPlainMapping->GetFieldId (field);
        while (true)
        {
            if (topCondition && topCondition->untilField == fieldId)
            {
                PopVisibilityCondition ();
                topCondition = topCondition->popTo;
                continue;
            }

            if (nextCondition && nextCondition->sinceField == fieldId)
            {
                PushVisibilityCondition (ProjectNestedField (objectFieldId, nextCondition->sourceField),
                                         nextCondition->operation, nextCondition->argument);
                topCondition = nextCondition;
                nextCondition = nextCondition->next;
            }

            break;
        }

        const char *fullName = EMERGENCE_BUILD_STRING (_name, PROJECTION_NAME_SEPARATOR, field.GetName ());
        [[maybe_unused]] FieldId nestedFieldId = 0u;

        switch (field.GetArchetype ())
        {
        case FieldArchetype::BIT:
            nestedFieldId = state.AddField (FieldData::BitSeed {
                Memory::UniqueString {fullName}, _offset + field.GetOffset (), field.GetBitOffset (), true});
            break;

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
            nestedFieldId =
                state.AddField (FieldData::StandardSeed {Memory::UniqueString {fullName}, field.GetArchetype (), true,
                                                         _offset + field.GetOffset (), field.GetSize ()});
            break;

        case FieldArchetype::UNIQUE_STRING:
            nestedFieldId = state.AddField (
                FieldData::UniqueStringSeed {Memory::UniqueString {fullName}, _offset + field.GetOffset (), true});
            break;

        case FieldArchetype::NESTED_OBJECT:
            // We don't need to recursively add fields, because given nested mapping is finished,
            // therefore all fields of internal objects are already projected into this mapping.
            nestedFieldId = state.AddField (FieldData::NestedObjectSeed {Memory::UniqueString {fullName},
                                                                         _offset + field.GetOffset (),
                                                                         field.GetNestedObjectMapping ().Get (), true});
            break;
        }

        assert (nestedFieldId == ProjectNestedField (objectFieldId, fieldId));
    }

    return objectFieldId;
}

void MappingBuilder::PushVisibilityCondition (FieldId _field,
                                              ConditionalOperation _operation,
                                              std::uint64_t _argument) noexcept
{
    block_cast<PlainMappingBuilder> (data).PushCondition (_field, _operation, _argument);
}

void MappingBuilder::PopVisibilityCondition () noexcept
{
    block_cast<PlainMappingBuilder> (data).PopCondition ();
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
