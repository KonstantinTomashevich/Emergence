#include <cassert>

#include <StandardLayout/MappingBuilder.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
static constexpr std::size_t INITIAL_FIELD_CAPACITY = 16u;

struct MappingBuilderState final
{
    ~MappingBuilderState ()
    {
        free (mappingUnderConstruction);
    }

    FieldId AddField (const FieldMeta &_fieldMeta)
    {
        assert (mappingUnderConstruction);
        assert (mappingUnderConstruction->fieldCount <= fieldCapacity);

        assert (_fieldMeta.size != 0u || _fieldMeta.archetype == FieldArchetype::BIT);
        assert (_fieldMeta.archetype != FieldArchetype::BIT || _fieldMeta.bitOffset < 8u);
        assert (_fieldMeta.offset + (_fieldMeta.archetype == FieldArchetype::BIT ? 1u : _fieldMeta.size) <=
                mappingUnderConstruction->objectSize);

        if (mappingUnderConstruction->fieldCount == fieldCapacity)
        {
            // Use default x2 scaling strategy, assert that there will be no overflow.
            assert (fieldCapacity * 2u > fieldCapacity);
            fieldCapacity *= 2u;
            mappingUnderConstruction = mappingUnderConstruction->Reallocate (fieldCapacity);
        }

        FieldId fieldId = mappingUnderConstruction->fieldCount;
        ++mappingUnderConstruction->fieldCount;
        *mappingUnderConstruction->GetField (fieldId) = _fieldMeta;
        return fieldId;
    }

    PlainMapping *mappingUnderConstruction = nullptr;
    std::size_t fieldCapacity = 0u;
};

FieldMeta ConstructBitField (std::size_t _offset, uint_fast8_t _bitOffset)
{
    FieldMeta meta;
    meta.offset = _offset;
    meta.bitOffset = _bitOffset;
    meta.archetype = FieldArchetype::BIT;
    return meta;
}

FieldMeta ConstructStandardField (std::size_t _offset, std::size_t _size, FieldArchetype _archetype)
{
    assert (_archetype != FieldArchetype::BIT);
    FieldMeta meta;
    meta.offset = _offset;
    meta.size = _size;
    meta.archetype = _archetype;
    return meta;
}

MappingBuilder::MappingBuilder () noexcept
{
    handle = new MappingBuilderState ();
}

MappingBuilder::~MappingBuilder ()
{
    assert (handle);
    delete static_cast <MappingBuilderState *> (handle);
}

void MappingBuilder::Begin (std::size_t _objectSize) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    assert (!state->mappingUnderConstruction);

    state->fieldCapacity = INITIAL_FIELD_CAPACITY;
    state->mappingUnderConstruction = PlainMapping::Allocate (state->fieldCapacity);
    state->mappingUnderConstruction->objectSize = _objectSize;
}

Mapping MappingBuilder::End () noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    assert (state->mappingUnderConstruction);

    PlainMapping *mapping = state->mappingUnderConstruction;
    state->mappingUnderConstruction = nullptr;

    // Use reallocation to get rid of unused memory, which was reserved for additional fields.
    return Mapping (mapping->Reallocate (mapping->fieldCount));
}

FieldId MappingBuilder::RegisterBit (std::size_t _offset, uint_fast8_t _bitOffset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructBitField (_offset, _bitOffset));
}

FieldId MappingBuilder::RegisterInt8 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (int8_t), FieldArchetype::INT));
}

FieldId MappingBuilder::RegisterInt16 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (int16_t), FieldArchetype::INT));
}

FieldId MappingBuilder::RegisterInt32 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (int32_t), FieldArchetype::INT));
}

FieldId MappingBuilder::RegisterInt64 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (int64_t), FieldArchetype::INT));
}

FieldId MappingBuilder::RegisterUInt8 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (uint8_t), FieldArchetype::UINT));
}

FieldId MappingBuilder::RegisterUInt16 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (uint16_t), FieldArchetype::UINT));
}

FieldId MappingBuilder::RegisterUInt32 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (uint32_t), FieldArchetype::UINT));
}

FieldId MappingBuilder::RegisterUInt64 (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (uint64_t), FieldArchetype::UINT));
}

FieldId MappingBuilder::RegisterFloat (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (float), FieldArchetype::FLOAT));
}

FieldId MappingBuilder::RegisterDouble (std::size_t _offset) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, sizeof (double), FieldArchetype::FLOAT));
}

FieldId MappingBuilder::RegisterString (std::size_t _offset, std::size_t _maxSize) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, _maxSize, FieldArchetype::STRING));
}

FieldId MappingBuilder::RegisterBlock (std::size_t _offset, std::size_t _size) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);
    return state->AddField (ConstructStandardField (_offset, _size, FieldArchetype::BLOCK));
}

FieldId MappingBuilder::RegisterNestedObject (std::size_t _offset, const Mapping &nestedMapping) noexcept
{
    assert (handle);
    auto *state = static_cast <MappingBuilderState *> (handle);

    assert (nestedMapping.handle);
    const auto *nestedPlainMapping = static_cast <const PlainMapping *> (nestedMapping.handle);

    FieldId mainNestedId = state->AddField (
        ConstructStandardField (_offset, nestedPlainMapping->objectSize, FieldArchetype::BLOCK));

    // TODO: Better iteration mechanism when mapping iteration support will be added?
    for (FieldId field = 0u; field < static_cast <const PlainMapping *> (nestedMapping.handle)->fieldCount; ++field)
    {
        const FieldMeta *fieldMeta = nestedPlainMapping->GetField (field);
        assert (fieldMeta);

        FieldId nestedFieldId = state->AddField (
            fieldMeta->archetype == FieldArchetype::BIT ?
            ConstructBitField (_offset + fieldMeta->offset, fieldMeta->bitOffset) :
            ConstructStandardField (_offset + fieldMeta->offset, fieldMeta->size, fieldMeta->archetype));

        assert (nestedFieldId == ProjectNestedField (mainNestedId, field));
    }

    return mainNestedId;
}
} // namespace Emergence::StandardLayout