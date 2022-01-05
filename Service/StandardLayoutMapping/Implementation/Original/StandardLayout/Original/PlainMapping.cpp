#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <cstdlib>
#include <new>

#include <API/Common/Implementation/Iterator.hpp>

#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
FieldArchetype FieldData::GetArchetype () const noexcept
{
    return archetype;
}

size_t FieldData::GetOffset () const noexcept
{
    return offset;
}

size_t FieldData::GetSize () const noexcept
{
    return size;
}

uint_fast8_t FieldData::GetBitOffset () const noexcept
{
    assert (archetype == FieldArchetype::BIT);
    return bitOffset;
}

Handling::Handle<PlainMapping> FieldData::GetNestedObjectMapping () const noexcept
{
    assert (archetype == FieldArchetype::NESTED_OBJECT);
    return nestedObjectMapping;
}

Memory::UniqueString FieldData::GetName () const noexcept
{
    return name;
}

// NOLINTNEXTLINE(modernize-use-equals-default): Default constructor is implicitly deleted.
FieldData::FieldData () noexcept
{
}

FieldData::FieldData (FieldData::StandardSeed _seed) noexcept
    : archetype (_seed.archetype),
      offset (_seed.offset),
      size (_seed.size),
      name (_seed.name)
{
    assert (archetype != FieldArchetype::BIT);
    assert (archetype != FieldArchetype::UNIQUE_STRING);
    assert (archetype != FieldArchetype::NESTED_OBJECT);
}

FieldData::FieldData (FieldData::BitSeed _seed) noexcept
    : archetype (FieldArchetype::BIT),
      offset (_seed.offset),
      size (1u),
      name (_seed.name),
      bitOffset (_seed.bitOffset)
{
}

FieldData::FieldData (FieldData::UniqueStringSeed _seed) noexcept
    : archetype (FieldArchetype::UNIQUE_STRING),
      offset (_seed.offset),
      size (sizeof (Memory::UniqueString)),
      name (_seed.name)
{
}

FieldData::FieldData (FieldData::NestedObjectSeed _seed) noexcept
    : archetype (FieldArchetype::NESTED_OBJECT),
      offset (_seed.offset),
      name (_seed.name),
      nestedObjectMapping (std::move (_seed.nestedObjectMapping))
{
    assert (nestedObjectMapping);
    size = nestedObjectMapping->GetObjectSize ();
}

FieldData::~FieldData ()
{
    if (archetype == FieldArchetype::NESTED_OBJECT)
    {
        nestedObjectMapping.~Handle ();
    }
}

std::size_t PlainMapping::GetObjectSize () const noexcept
{
    return objectSize;
}

std::size_t PlainMapping::GetFieldCount () const noexcept
{
    return fieldCount;
}

Memory::UniqueString PlainMapping::GetName () const noexcept
{
    return name;
}

const FieldData *PlainMapping::GetField (FieldId _field) const noexcept
{
    if (_field < fieldCount)
    {
        return &fields[_field];
    }

    return nullptr;
}

const FieldData *PlainMapping::Begin () const noexcept
{
    return &fields[0u];
}

const FieldData *PlainMapping::End () const noexcept
{
    return &fields[fieldCount];
}

FieldId PlainMapping::GetFieldId (const FieldData &_field) const
{
    assert (&_field >= Begin ());
    assert (&_field < End ());
    return &_field - Begin ();
}

Memory::Heap PlainMapping::heap {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Root (),
                                                                    Memory::UniqueString {"PlainMapping"}}};

std::size_t PlainMapping::CalculateMappingSize (std::size_t _fieldCapacity) noexcept
{
    return sizeof (PlainMapping) + _fieldCapacity * sizeof (FieldData);
}

PlainMapping::PlainMapping (Memory::UniqueString _name, std::size_t _objectSize) noexcept
    : objectSize (_objectSize),
      name (_name)
{
    assert (objectSize > 0u);
}

PlainMapping::~PlainMapping () noexcept
{
    for (const FieldData &fieldData : *this)
    {
        fieldData.~FieldData ();
    }
}

void *PlainMapping::operator new (std::size_t /*unused*/, std::size_t _fieldCapacity) noexcept
{
    return heap.Acquire (CalculateMappingSize (_fieldCapacity));
}

void PlainMapping::operator delete (void *_pointer) noexcept
{
    heap.Release (_pointer, CalculateMappingSize (static_cast<PlainMapping *> (_pointer)->fieldCapacity));
}

PlainMapping *PlainMapping::ChangeCapacity (std::size_t _newFieldCapacity) noexcept
{
    assert (_newFieldCapacity >= fieldCount);
    auto *newInstance = static_cast<PlainMapping *> (
        heap.Resize (this, CalculateMappingSize (fieldCapacity), CalculateMappingSize (_newFieldCapacity)));

    newInstance->fieldCapacity = _newFieldCapacity;
    return newInstance;
}

const FieldData *begin (const PlainMapping &_mapping) noexcept
{
    return _mapping.Begin ();
}

const FieldData *end (const PlainMapping &_mapping) noexcept
{
    return _mapping.End ();
}

PlainMappingBuilder::~PlainMappingBuilder ()
{
    delete underConstruction;
}

void PlainMappingBuilder::Begin (Memory::UniqueString _name, std::size_t _objectSize) noexcept
{
    assert (!underConstruction);
    underConstruction = new (INITIAL_FIELD_CAPACITY) PlainMapping (_name, _objectSize);
    underConstruction->fieldCapacity = INITIAL_FIELD_CAPACITY;
}

Handling::Handle<PlainMapping> PlainMappingBuilder::End () noexcept
{
    assert (underConstruction);
    PlainMapping *finished = underConstruction->ChangeCapacity (underConstruction->fieldCount);
    underConstruction = nullptr;
    return finished;
}

std::pair<FieldId, FieldData *> PlainMappingBuilder::AllocateField () noexcept
{
    assert (underConstruction);
    assert (underConstruction->fieldCount <= underConstruction->fieldCapacity);

    if (underConstruction->fieldCount == underConstruction->fieldCapacity)
    {
        underConstruction = underConstruction->ChangeCapacity (underConstruction->fieldCapacity * 2u);
    }

    FieldId fieldId = underConstruction->fieldCount;
    ++underConstruction->fieldCount;
    return {fieldId, &underConstruction->fields[fieldId]};
}
} // namespace Emergence::StandardLayout
