#include <cassert>
#include <cstdlib>
#include <new>

#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
FieldArchetype FieldData::GetArchetype () const
{
    return archetype;
}

size_t FieldData::GetOffset () const
{
    return offset;
}

size_t FieldData::GetSize () const
{
    return size;
}

uint_fast8_t FieldData::GetBitOffset () const
{
    assert (archetype == FieldArchetype::BIT);
    return bitOffset;
}

Handling::Handle <PlainMapping> FieldData::GetNestedObjectMapping () const
{
    assert (archetype == FieldArchetype::NESTED_OBJECT);
    return nestedObjectMapping;
}

FieldData::FieldData (FieldData::StandardSeed _seed) noexcept
    : archetype (_seed.archetype),
      offset (_seed.offset),
      size (_seed.size)
{
    assert (archetype != FieldArchetype::BIT);
    assert (archetype != FieldArchetype::NESTED_OBJECT);
}

FieldData::FieldData (FieldData::BitSeed _seed) noexcept
    : archetype (FieldArchetype::BIT),
      offset (_seed.offset),
      size (1u),
      bitOffset (_seed.bitOffset)
{
}

FieldData::FieldData (FieldData::NestedObjectSeed _seed) noexcept
    : archetype (FieldArchetype::NESTED_OBJECT),
      offset (_seed.offset),
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

PlainMapping::ConstIterator::ConstIterator () noexcept
    : target (nullptr)
{
}

PlainMapping::ConstIterator::ConstIterator (const FieldData *_target) noexcept
    : target (_target)
{
}

const FieldData &PlainMapping::ConstIterator::operator * () const noexcept
{
    assert (target);
    return *target;
}

const FieldData *PlainMapping::ConstIterator::operator -> () const noexcept
{
    return target;
}

PlainMapping::ConstIterator &PlainMapping::ConstIterator::operator ++ () noexcept
{
    return *this += 1u;
}

PlainMapping::ConstIterator PlainMapping::ConstIterator::operator ++ (int) noexcept
{
    ConstIterator beforeIncrement (target);
    *this += 1u;
    return beforeIncrement;
}

PlainMapping::ConstIterator &PlainMapping::ConstIterator::operator -- () noexcept
{
    return *this -= 1u;
}

PlainMapping::ConstIterator PlainMapping::ConstIterator::operator -- (int) noexcept
{
    ConstIterator beforeDecrement (target);
    *this -= 1u;
    return beforeDecrement;
}

PlainMapping::ConstIterator PlainMapping::ConstIterator::operator + (ptrdiff_t _steps) const noexcept
{
    return PlainMapping::ConstIterator (target + _steps);
}

PlainMapping::ConstIterator operator + (ptrdiff_t _steps, const PlainMapping::ConstIterator &_iterator) noexcept
{
    return _iterator + _steps;
}

PlainMapping::ConstIterator &PlainMapping::ConstIterator::operator += (ptrdiff_t _steps) noexcept
{
    target += _steps;
    return *this;
}

PlainMapping::ConstIterator PlainMapping::ConstIterator::operator - (ptrdiff_t _steps) const noexcept
{
    return PlainMapping::ConstIterator (target - _steps);
}

PlainMapping::ConstIterator &PlainMapping::ConstIterator::operator -= (ptrdiff_t _steps) noexcept
{
    target -= _steps;
    return *this;
}

const FieldData &PlainMapping::ConstIterator::operator [] (std::size_t _index) const noexcept
{
    assert (target);
    return target[_index];
}

ptrdiff_t PlainMapping::ConstIterator::operator - (const PlainMapping::ConstIterator &_other) const noexcept
{
    return target - _other.target;
}

bool PlainMapping::ConstIterator::operator == (const PlainMapping::ConstIterator &_other) const noexcept
{
    return target == _other.target;
}

bool PlainMapping::ConstIterator::operator != (const PlainMapping::ConstIterator &_other) const noexcept
{
    return !(*this == _other);
}

bool PlainMapping::ConstIterator::operator < (const PlainMapping::ConstIterator &_other) const noexcept
{
    return target < _other.target;
}

bool PlainMapping::ConstIterator::operator > (const PlainMapping::ConstIterator &_other) const noexcept
{
    return _other < *this;
}

bool PlainMapping::ConstIterator::operator <= (const PlainMapping::ConstIterator &_other) const noexcept
{
    return !(_other < *this);
}

bool PlainMapping::ConstIterator::operator >= (const PlainMapping::ConstIterator &_other) const noexcept
{
    return !(*this < _other);
}

std::size_t PlainMapping::GetObjectSize () const noexcept
{
    return objectSize;
}

std::size_t PlainMapping::GetFieldCount () const noexcept
{
    return fieldCount;
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
    return const_cast <FieldData *> (const_cast <const PlainMapping *> (this)->GetField (_field));
}

PlainMapping::ConstIterator PlainMapping::Begin () const noexcept
{
    return PlainMapping::ConstIterator (&fields[0u]);
}

PlainMapping::ConstIterator PlainMapping::End () const noexcept
{
    return PlainMapping::ConstIterator (&fields[fieldCount]);
}

FieldId PlainMapping::GetFieldId (const PlainMapping::ConstIterator &_iterator) const
{
    assert(_iterator >= Begin ());
    assert(_iterator < End ());
    return _iterator - Begin ();
}

FieldId PlainMapping::GetFieldId (const FieldData &_field) const
{
    return GetFieldId (ConstIterator (&_field));
}

std::size_t PlainMapping::CalculateMappingSize (std::size_t _fieldCapacity) noexcept
{
    return sizeof (PlainMapping) + _fieldCapacity * sizeof (FieldData);
}

PlainMapping::PlainMapping (std::size_t _objectSize) noexcept
    : objectSize (_objectSize)
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
    return static_cast <PlainMapping *> (realloc (this, CalculateMappingSize (_newFieldCapacity)));
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

void PlainMappingBuilder::Begin (std::size_t _objectSize) noexcept
{
    assert (!underConstruction);
    fieldCapacity = INITIAL_FIELD_CAPACITY;
    underConstruction = new (fieldCapacity) PlainMapping (_objectSize);
}

Handling::Handle <PlainMapping> PlainMappingBuilder::End () noexcept
{
    assert (underConstruction);
    ReallocateMapping (underConstruction->fieldCount);

    PlainMapping *finished = underConstruction;
    underConstruction = nullptr;
    return finished;
}

FieldId PlainMappingBuilder::AddField (FieldData::StandardSeed _seed) noexcept
{
    auto[fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

FieldId PlainMappingBuilder::AddField (FieldData::BitSeed _seed) noexcept
{
    auto[fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

FieldId PlainMappingBuilder::AddField (FieldData::NestedObjectSeed _seed) noexcept
{
    auto[fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    assert (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}

std::pair <FieldId, FieldData *> PlainMappingBuilder::AllocateField () noexcept
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
