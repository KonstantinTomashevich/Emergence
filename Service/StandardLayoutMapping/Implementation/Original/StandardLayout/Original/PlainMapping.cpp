#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <cstdlib>
#include <new>

#include <API/Common/Implementation/Iterator.hpp>

#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
static Memory::Profiler::AllocationGroup GetAllocationGroup ()
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (),
                                                    Memory::UniqueString {"PlainMapping"}};
    return group;
}

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

Memory::OrderedPool &ConditionData::GetPool () noexcept
{
    static Memory::OrderedPool pool {GetAllocationGroup (), sizeof (ConditionData), alignof (ConditionData)};
    return pool;
}

const FieldData *PlainMapping::ConditionalFieldIterator::operator* () const noexcept
{
    return currentField;
}

PlainMapping::ConditionalFieldIterator &PlainMapping::ConditionalFieldIterator::operator= (
    const ConditionalFieldIterator &_other) noexcept = default;

PlainMapping::ConditionalFieldIterator &PlainMapping::ConditionalFieldIterator::operator= (
    ConditionalFieldIterator &&_other) noexcept = default;

PlainMapping::ConditionalFieldIterator::ConditionalFieldIterator (const ConditionalFieldIterator &_other) noexcept =
    default;

PlainMapping::ConditionalFieldIterator::ConditionalFieldIterator (ConditionalFieldIterator &&_other) noexcept = default;

PlainMapping::ConditionalFieldIterator::~ConditionalFieldIterator () noexcept = default;

PlainMapping::ConditionalFieldIterator &PlainMapping::ConditionalFieldIterator::operator++ () noexcept
{
    do
    {
        ++currentField;
        UpdateCondition ();
    } while (!IsConditionSatisfied ());

    return *this;
}

PlainMapping::ConditionalFieldIterator PlainMapping::ConditionalFieldIterator::operator++ (int) noexcept
{
    ConditionalFieldIterator other = *this;
    ++*this;
    return other;
}

bool PlainMapping::ConditionalFieldIterator::operator== (
    const PlainMapping::ConditionalFieldIterator &_other) const noexcept
{
    return currentField == _other.currentField;
}

bool PlainMapping::ConditionalFieldIterator::operator!= (
    const PlainMapping::ConditionalFieldIterator &_other) const noexcept
{
    return !(*this == _other);
}

PlainMapping::ConditionalFieldIterator::ConditionalFieldIterator (const PlainMapping *_owner,
                                                                  const FieldData *_field,
                                                                  const void *_object) noexcept
    : object (_object),
      currentField (_field),
      nextCondition (_owner->firstCondition)
{
    assert (currentField == _owner->Begin () || currentField == _owner->End ());
    if (currentField == _owner->Begin ())
    {
        UpdateCondition ();
        if (!IsConditionSatisfied ())
        {
            ++*this;
        }
    }
}

void PlainMapping::ConditionalFieldIterator::UpdateCondition () noexcept
{
    while (true)
    {
        if (topCondition && topCondition->untilField == currentField)
        {
            topCondition = topCondition->popTo;
            continue;
        }

        if (nextCondition && nextCondition->sinceField == currentField)
        {
            assert (nextCondition->popTo == topCondition);
            topCondition = nextCondition;
            nextCondition = nextCondition->next;
            continue;
        }

        return;
    }
}

bool PlainMapping::ConditionalFieldIterator::IsConditionSatisfied () const noexcept
{
    if (topCondition)
    {
        const auto *shifted = static_cast<const uint8_t *> (object) + topCondition->sourceField->GetOffset ();

#define DO_OPERATION(Operation)                                                                                        \
    switch (topCondition->sourceField->GetSize ())                                                                     \
    {                                                                                                                  \
    case 1u:                                                                                                           \
        return static_cast<uint64_t> (*shifted) < topCondition->argument;                                              \
                                                                                                                       \
    case 2u:                                                                                                           \
        return static_cast<uint64_t> (*reinterpret_cast<const uint16_t *> (shifted)) < topCondition->argument;         \
                                                                                                                       \
    case 4u:                                                                                                           \
        return static_cast<uint64_t> (*reinterpret_cast<const uint32_t *> (shifted)) < topCondition->argument;         \
                                                                                                                       \
    case 8u:                                                                                                           \
        return *reinterpret_cast<const uint64_t *> (shifted) < topCondition->argument;                                 \
                                                                                                                       \
    default:                                                                                                           \
        assert (false);                                                                                                \
        return true;                                                                                                   \
    }

        switch (topCondition->operation)
        {
        case ConditionalOperation::EQUAL:
            DO_OPERATION (==)

        case ConditionalOperation::LESS:
            DO_OPERATION (<)

        case ConditionalOperation::GREATER:
            DO_OPERATION (>)
        }
    }

    return true;
}

std::size_t PlainMapping::GetObjectSize () const noexcept
{
    return objectSize;
}

std::size_t PlainMapping::GetObjectAlignment () const noexcept
{
    return objectAlignment;
}

std::size_t PlainMapping::GetFieldCount () const noexcept
{
    return fieldCount;
}

Memory::UniqueString PlainMapping::GetName () const noexcept
{
    return name;
}

void PlainMapping::Construct (void *_address) const noexcept
{
    if (constructor)
    {
        constructor (_address);
    }
}

void PlainMapping::Destruct (void *_address) const noexcept
{
    if (destructor)
    {
        destructor (_address);
    }
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

PlainMapping::ConditionalFieldIterator PlainMapping::BeginConditional (const void *_object) const noexcept
{
    return {this, Begin (), _object};
}

PlainMapping::ConditionalFieldIterator PlainMapping::EndConditional () const noexcept
{
    return {this, End (), nullptr};
}

FieldId PlainMapping::GetFieldId (const FieldData &_field) const
{
    assert (&_field >= Begin ());
    assert (&_field < End ());
    return &_field - Begin ();
}

Memory::Heap &PlainMapping::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}

std::size_t PlainMapping::CalculateMappingSize (std::size_t _fieldCapacity) noexcept
{
    return sizeof (PlainMapping) + _fieldCapacity * sizeof (FieldData);
}

PlainMapping::PlainMapping (Memory::UniqueString _name, std::size_t _objectSize, std::size_t _objectAlignment) noexcept
    : objectSize (_objectSize),
      objectAlignment (_objectAlignment),
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

    ConditionData *condition = firstCondition;
    while (condition)
    {
        ConditionData *next = condition;
        ConditionData::GetPool ().Release (condition);
        condition->~ConditionData ();
        condition = next;
    }
}

void *PlainMapping::operator new (std::size_t /*unused*/, std::size_t _fieldCapacity) noexcept
{
    return GetHeap ().Acquire (CalculateMappingSize (_fieldCapacity), alignof (PlainMapping));
}

void PlainMapping::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, CalculateMappingSize (static_cast<PlainMapping *> (_pointer)->fieldCapacity));
}

PlainMapping *PlainMapping::ChangeCapacity (std::size_t _newFieldCapacity) noexcept
{
    assert (_newFieldCapacity >= fieldCount);
    auto *newInstance = static_cast<PlainMapping *> (GetHeap ().Resize (
        this, alignof (PlainMapping), CalculateMappingSize (fieldCapacity), CalculateMappingSize (_newFieldCapacity)));

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

PlainMappingBuilder::PlainMappingBuilder (PlainMappingBuilder &&_other) noexcept
    : underConstruction (_other.underConstruction)
{
    _other.underConstruction = nullptr;
}

PlainMappingBuilder::~PlainMappingBuilder ()
{
    delete underConstruction;
}

void PlainMappingBuilder::Begin (Memory::UniqueString _name,
                                 std::size_t _objectSize,
                                 std::size_t _objectAlignment) noexcept
{
    assert (!underConstruction);
    underConstruction = new (INITIAL_FIELD_CAPACITY) PlainMapping (_name, _objectSize, _objectAlignment);
    underConstruction->fieldCapacity = INITIAL_FIELD_CAPACITY;
}

Handling::Handle<PlainMapping> PlainMappingBuilder::End () noexcept
{
    assert (underConstruction);
    PlainMapping *finished = underConstruction->ChangeCapacity (underConstruction->fieldCount);
    underConstruction = nullptr;
    return finished;
}

void PlainMappingBuilder::SetConstructor (void (*_constructor) (void *)) noexcept
{
    assert (underConstruction);
    underConstruction->constructor = _constructor;
}

void PlainMappingBuilder::SetDestructor (void (*_destructor) (void *)) noexcept
{
    assert (underConstruction);
    underConstruction->destructor = _destructor;
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

void PlainMappingBuilder::PushCondition (FieldId _sourceField,
                                         ConditionalOperation _operation,
                                         std::uint64_t _argument) noexcept
{
    assert (_sourceField < underConstruction->fieldCount);
    assert (underConstruction->GetField (_sourceField)->archetype == FieldArchetype::UINT);

    auto *condition = new (ConditionData::GetPool ().Acquire ()) ConditionData {};
    condition->sinceField = underConstruction->End ();
    condition->sourceField = underConstruction->GetField (_sourceField);
    condition->operation = _operation;
    condition->argument = _argument;

    if (lastCondition)
    {
        lastCondition->next = condition;
    }
    else
    {
        underConstruction->firstCondition = condition;
    }

    lastCondition = condition;
    condition->popTo = topCondition;
    topCondition = condition;
}

void PlainMappingBuilder::PopCondition () noexcept
{
    assert (topCondition);
    topCondition->untilField = underConstruction->End ();
    topCondition = topCondition->popTo;
}
} // namespace Emergence::StandardLayout
