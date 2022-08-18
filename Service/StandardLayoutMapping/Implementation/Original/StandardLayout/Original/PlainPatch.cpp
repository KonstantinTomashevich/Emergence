#include <cassert>

#include <StandardLayout/Original/PlainPatch.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
Mapping PlainPatch::GetTypeMapping () const noexcept
{
    return mapping;
}

void PlainPatch::Apply (void *_object) const noexcept
{
    for (std::size_t index = 0u; index < valueCount; ++index)
    {
        const ValueSetter &setter = valueSetters[index];
        Field field = mapping.GetField (setter.field);
        uint8_t *address = static_cast<uint8_t *> (_object) + field.GetOffset ();

        switch (field.GetArchetype ())
        {
        case FieldArchetype::BIT:
            if (block_cast<bool> (setter.value))
            {
                *address |= 1u << field.GetBitOffset ();
            }
            else
            {
                *address &= ~(1u << field.GetBitOffset ());
            }
            break;

        case FieldArchetype::INT:
            switch (field.GetSize ())
            {
            case 1u:
                *reinterpret_cast<int8_t *> (address) = block_cast<int8_t> (setter.value);
                break;

            case 2u:
                *reinterpret_cast<int16_t *> (address) = block_cast<int16_t> (setter.value);
                break;

            case 4u:
                *reinterpret_cast<int32_t *> (address) = block_cast<int32_t> (setter.value);
                break;

            case 8u:
                *reinterpret_cast<int64_t *> (address) = block_cast<int64_t> (setter.value);
                break;
            }
            break;

        case FieldArchetype::UINT:
            switch (field.GetSize ())
            {
            case 1u:
                *address = block_cast<uint8_t> (setter.value);
                break;

            case 2u:
                *reinterpret_cast<uint16_t *> (address) = block_cast<uint16_t> (setter.value);
                break;

            case 4u:
                *reinterpret_cast<uint32_t *> (address) = block_cast<uint32_t> (setter.value);
                break;

            case 8u:
                *reinterpret_cast<uint64_t *> (address) = block_cast<uint64_t> (setter.value);
                break;
            }
            break;

        case FieldArchetype::FLOAT:
            switch (field.GetSize ())
            {
            case 4u:
                *reinterpret_cast<float *> (address) = block_cast<float> (setter.value);
                break;

            case 8u:
                *reinterpret_cast<double *> (address) = block_cast<double> (setter.value);
                break;
            }
            break;

        case FieldArchetype::UNIQUE_STRING:
            *reinterpret_cast<Memory::UniqueString *> (address) = block_cast<Memory::UniqueString> (setter.value);
            break;

        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
        case FieldArchetype::NESTED_OBJECT:
            // Unsupported!
            assert (false);
            break;
        }
    }
}

std::size_t PlainPatch::GetValueCount () const noexcept
{
    return valueCount;
}

const ValueSetter *PlainPatch::Begin () const noexcept
{
    return &valueSetters[0u];
}

const ValueSetter *PlainPatch::End () const noexcept
{
    return &valueSetters[valueCount];
}

Memory::Heap &PlainPatch::GetHeap () noexcept
{
    static Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"PlainPatch"}}};
    return heap;
}

std::size_t PlainPatch::CalculatePatchSize (std::size_t _valueCapacity) noexcept
{
    return sizeof (PlainPatch) + sizeof (ValueSetter) * _valueCapacity;
}

PlainPatch::PlainPatch (Mapping _mapping) noexcept
    : mapping (std::move (_mapping))
{
}

void *PlainPatch::operator new (std::size_t /*unused*/, std::size_t _valueCapacity) noexcept
{
    return GetHeap ().Acquire (CalculatePatchSize (_valueCapacity), alignof (PlainPatch));
}

void PlainPatch::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, CalculatePatchSize (static_cast<PlainPatch *> (_pointer)->valueCapacity));
}

PlainPatch *PlainPatch::ChangeCapacity (std::size_t _newValueCapacity) noexcept
{
    assert (_newValueCapacity >= valueCount);

    // We can safely use resize because we know how Mapping movement works in this implementation.
    auto *newInstance = static_cast<PlainPatch *> (GetHeap ().Resize (
        this, alignof (PlainPatch), CalculatePatchSize (valueCapacity), CalculatePatchSize (_newValueCapacity)));

    newInstance->valueCapacity = _newValueCapacity;
    return newInstance;
}

PlainPatchBuilder::PlainPatchBuilder (PlainPatchBuilder &&_other) noexcept
    : underConstruction (_other.underConstruction)
{
    _other.underConstruction = nullptr;
}

PlainPatchBuilder::~PlainPatchBuilder () noexcept
{
    delete underConstruction;
}

void PlainPatchBuilder::Begin (Mapping _mapping) noexcept
{
    assert (!underConstruction);
    underConstruction = new (INITIAL_VALUE_CAPACITY) PlainPatch (std::move (_mapping));
    underConstruction->valueCapacity = INITIAL_VALUE_CAPACITY;
}

void PlainPatchBuilder::Set (FieldId _field, const std::array<uint8_t, VALUE_MAX_SIZE> &_value) noexcept
{
    assert (underConstruction);
    if (underConstruction->valueCount == underConstruction->valueCapacity)
    {
        underConstruction = underConstruction->ChangeCapacity (underConstruction->valueCount * 2u);
    }

    underConstruction->valueSetters[underConstruction->valueCount] = {_field, _value};
    ++underConstruction->valueCount;
}

PlainPatch *PlainPatchBuilder::End () noexcept
{
    assert (underConstruction);
    PlainPatch *result = underConstruction->ChangeCapacity (underConstruction->valueCount);
    underConstruction = nullptr;
    return result;
}
} // namespace Emergence::StandardLayout
