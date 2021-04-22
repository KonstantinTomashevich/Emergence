#include <cassert>
#include <cstdlib>
#include <new>

#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
static std::size_t CalculateMappingSize (std::size_t _fieldCapacity) noexcept
{
    return sizeof (PlainMapping) + _fieldCapacity * sizeof (FieldData);
}

FieldData::FieldData (FieldArchetype _archetype, size_t _offset, size_t _size)
    : archetype (_archetype),
      offset (_offset),
      size (_size)
{
    assert (archetype != FieldArchetype::BIT);
    assert (archetype != FieldArchetype::NESTED_OBJECT);
}

FieldData::FieldData (size_t _offset, uint_fast8_t _bitOffset)
    : archetype (FieldArchetype::BIT),
      offset (_offset),
      size (1u),
      bitOffset (_bitOffset)
{
}

FieldData::FieldData (size_t _offset, PlainMapping *_nestedObjectMapping)
    : archetype (FieldArchetype::NESTED_OBJECT),
      offset (_offset),
      nestedObjectMapping (_nestedObjectMapping)
{
    assert (_nestedObjectMapping);
    nestedObjectMapping->RegisterReference ();
    size = nestedObjectMapping->GetObjectSize ();
}

FieldData::FieldData (const FieldData &_other)
{
    // TODO: Placement new looks awkward here.
    switch (_other.archetype)
    {
        case FieldArchetype::BIT:
            new (this) FieldData (_other.offset, _other.bitOffset);
            break;

        case FieldArchetype::INT:
        case FieldArchetype::UINT:
        case FieldArchetype::FLOAT:
        case FieldArchetype::STRING:
        case FieldArchetype::BLOCK:
            new (this) FieldData (_other.archetype, _other.offset, _other.size);
            break;

        case FieldArchetype::NESTED_OBJECT:
            new (this) FieldData (_other.offset, _other.nestedObjectMapping);
            break;
    }
}

FieldData::~FieldData ()
{
    if (archetype == FieldArchetype::NESTED_OBJECT)
    {
        nestedObjectMapping->UnregisterReference ();
    }
}

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

PlainMapping *FieldData::GetNestedObjectMapping () const
{
    assert (archetype == FieldArchetype::NESTED_OBJECT);
    return nestedObjectMapping;
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

void PlainMapping::RegisterReference () noexcept
{
    assert (references + 1u > references);
    ++references;
}

void PlainMapping::UnregisterReference () noexcept
{
    assert (references > 0u);
    --references;

    if (references == 0u)
    {
        this->~PlainMapping ();
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

const FieldData *PlainMapping::GetField (FieldId _field) const noexcept
{
    if (_field < fieldCount)
    {
        return reinterpret_cast <const FieldData *> (this + 1u) + _field;
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
    return PlainMapping::ConstIterator (reinterpret_cast <const FieldData *> (this + 1u));
}

PlainMapping::ConstIterator PlainMapping::End () const noexcept
{
    return Begin () + fieldCount;
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

    free (this);
}

PlainMapping::ConstIterator begin (const PlainMapping &mapping) noexcept
{
    return mapping.Begin ();
}

PlainMapping::ConstIterator end (const PlainMapping &mapping) noexcept
{
    return mapping.End ();
}

void PlainMappingBuilder::Begin (std::size_t _objectSize) noexcept
{
    assert (underConstruction == nullptr);
    fieldCapacity = INITIAL_FIELD_CAPACITY;
    underConstruction = new (malloc (CalculateMappingSize (fieldCapacity))) PlainMapping (_objectSize);
}

PlainMapping *PlainMappingBuilder::End () noexcept
{
    assert (underConstruction);
    ReallocateMapping (underConstruction->fieldCount);

    PlainMapping *finished = underConstruction;
    underConstruction = nullptr;
    return finished;
}

FieldId PlainMappingBuilder::AddField (const FieldData &_fieldData) noexcept
{
    assert (underConstruction);
    assert (underConstruction->fieldCount <= fieldCapacity);
    assert (_fieldData.GetOffset () + _fieldData.GetSize () <= underConstruction->objectSize);

    if (underConstruction->fieldCount == fieldCapacity)
    {
        ReallocateMapping (fieldCapacity * 2u);
    }

    FieldId fieldId = underConstruction->fieldCount;
    ++underConstruction->fieldCount;

    FieldData *output = underConstruction->GetField (fieldId);
    assert (output);
    new (output) FieldData (_fieldData);
    return fieldId;
}

void PlainMappingBuilder::ReallocateMapping (std::size_t _fieldCapacity) noexcept
{
    assert (underConstruction);
    underConstruction = static_cast <PlainMapping *> (
        realloc (underConstruction, CalculateMappingSize (_fieldCapacity)));
    fieldCapacity = _fieldCapacity;
}
} // namespace Emergence::StandardLayout
