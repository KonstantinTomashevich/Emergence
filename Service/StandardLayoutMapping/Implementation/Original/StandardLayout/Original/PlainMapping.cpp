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

const char *FieldData::GetName () const noexcept
{
    return name;
}

FieldData::FieldData () noexcept : archetype (FieldArchetype::INT), offset (0u), size (0u), name {0u}
{
}

FieldData::FieldData (FieldData::StandardSeed _seed) noexcept
    : archetype (_seed.archetype),
      offset (_seed.offset),
      size (_seed.size)
{
    assert (archetype != FieldArchetype::BIT);
    assert (archetype != FieldArchetype::NESTED_OBJECT);
    CopyName (_seed.name);
}

FieldData::FieldData (FieldData::BitSeed _seed) noexcept
    : archetype (FieldArchetype::BIT),
      offset (_seed.offset),
      size (1u),
      bitOffset (_seed.bitOffset)
{
    CopyName (_seed.name);
}

FieldData::FieldData (FieldData::NestedObjectSeed _seed) noexcept
    : archetype (FieldArchetype::NESTED_OBJECT),
      offset (_seed.offset),
      nestedObjectMapping (std::move (_seed.nestedObjectMapping))
{
    assert (nestedObjectMapping);
    size = nestedObjectMapping->GetObjectSize ();
    CopyName (_seed.name);
}

FieldData::~FieldData ()
{
    if (archetype == FieldArchetype::NESTED_OBJECT)
    {
        nestedObjectMapping.~Handle ();
    }

    free (name);
}

void FieldData::CopyName (const char *_name) noexcept
{
    assert (_name);
    name = static_cast<char *> (malloc ((strlen (_name) + 1u) * sizeof (char)));
    strcpy (name, _name);
}

using ConstIterator = PlainMapping::ConstIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (ConstIterator, target)

PlainMapping::ConstIterator::ConstIterator () noexcept : target (nullptr)
{
}

const FieldData &PlainMapping::ConstIterator::operator* () const noexcept
{
    assert (target);
    return *target;
}

const FieldData *PlainMapping::ConstIterator::operator-> () const noexcept
{
    return target;
}

std::size_t PlainMapping::GetObjectSize () const noexcept
{
    return objectSize;
}

std::size_t PlainMapping::GetFieldCount () const noexcept
{
    return fieldCount;
}

const char *PlainMapping::GetName () const noexcept
{
    return name;
}

const FieldData *PlainMapping::GetField (FieldId _field) const noexcept
{
    if (_field < fieldCount)
    {
        return &fields[_field];
    }
    else
    {
        return nullptr;
    }
}

FieldData *PlainMapping::GetField (FieldId _field) noexcept
{
    return const_cast<FieldData *> (const_cast<const PlainMapping *> (this)->GetField (_field));
}

PlainMapping::ConstIterator PlainMapping::Begin () const noexcept
{
    return PlainMapping::ConstIterator (&fields[0u]);
}

PlainMapping::ConstIterator PlainMapping::End () const noexcept
{
    return PlainMapping::ConstIterator (&fields[fieldCount]);
}

FieldId PlainMapping::GetFieldId (const FieldData &_field) const
{
    assert (&_field >= Begin ().target);
    assert (&_field < End ().target);
    return &_field - Begin ().target;
}

std::size_t PlainMapping::CalculateMappingSize (std::size_t _fieldCapacity) noexcept
{
    return sizeof (PlainMapping) + _fieldCapacity * sizeof (FieldData);
}

PlainMapping::PlainMapping (const char *_name, std::size_t _objectSize) noexcept
    : objectSize (_objectSize),
      name (static_cast<char *> (malloc ((strlen (_name) + 1u) * sizeof (char))))
{
    assert (objectSize > 0u);
    assert (_name);
    strcpy (name, _name);
}

PlainMapping::~PlainMapping () noexcept
{
    free (name);
    for (const FieldData &fieldData : *this)
    {
        fieldData.~FieldData ();
    }
}

void *PlainMapping::operator new (std::size_t, std::size_t _fieldCapacity) noexcept
{
    return malloc (CalculateMappingSize (_fieldCapacity));
}

void PlainMapping::operator delete (void *_pointer) noexcept
{
    free (_pointer);
}

PlainMapping *PlainMapping::ChangeCapacity (std::size_t _newFieldCapacity) noexcept
{
    assert (_newFieldCapacity >= fieldCount);
    return static_cast<PlainMapping *> (realloc (this, CalculateMappingSize (_newFieldCapacity)));
}

PlainMapping::ConstIterator begin (const PlainMapping &mapping) noexcept
{
    return mapping.Begin ();
}

PlainMapping::ConstIterator end (const PlainMapping &mapping) noexcept
{
    return mapping.End ();
}

PlainMappingBuilder::~PlainMappingBuilder ()
{
    delete underConstruction;
}

void PlainMappingBuilder::Begin (const char *_name, std::size_t _objectSize) noexcept
{
    assert (!underConstruction);
    fieldCapacity = INITIAL_FIELD_CAPACITY;
    underConstruction = new (fieldCapacity) PlainMapping (_name, _objectSize);
}

Handling::Handle<PlainMapping> PlainMappingBuilder::End () noexcept
{
    assert (underConstruction);
    ReallocateMapping (underConstruction->fieldCount);

    PlainMapping *finished = underConstruction;
    underConstruction = nullptr;
    return finished;
}

FieldId PlainMappingBuilder::AddField (FieldData::StandardSeed _seed) noexcept
{
    auto [fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

FieldId PlainMappingBuilder::AddField (FieldData::BitSeed _seed) noexcept
{
    auto [fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

FieldId PlainMappingBuilder::AddField (FieldData::NestedObjectSeed _seed) noexcept
{
    auto [fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

std::pair<FieldId, FieldData *> PlainMappingBuilder::AllocateField () noexcept
{
    assert (underConstruction);
    assert (underConstruction->fieldCount <= fieldCapacity);

    if (underConstruction->fieldCount == fieldCapacity)
    {
        ReallocateMapping (fieldCapacity * 2u);
    }

    FieldId fieldId = underConstruction->fieldCount;
    ++underConstruction->fieldCount;

    FieldData *allocated = underConstruction->GetField (fieldId);
    assert (allocated);
    return {fieldId, allocated};
}

void PlainMappingBuilder::ReallocateMapping (std::size_t _fieldCapacity) noexcept
{
    assert (underConstruction);
    fieldCapacity = _fieldCapacity;
    underConstruction = underConstruction->ChangeCapacity (_fieldCapacity);
}
} // namespace Emergence::StandardLayout
