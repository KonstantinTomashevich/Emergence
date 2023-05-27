#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <new>

#include <Assert/Assert.hpp>

#include <API/Common/Implementation/Iterator.hpp>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Original/PlainMapping.hpp>
#include <StandardLayout/Patch.hpp>

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

bool FieldData::IsProjected () const noexcept
{
    return projected;
}

std::size_t FieldData::GetOffset () const noexcept
{
    return offset;
}

std::size_t FieldData::GetSize () const noexcept
{
    return size;
}

uint_fast8_t FieldData::GetBitOffset () const noexcept
{
    EMERGENCE_ASSERT (archetype == FieldArchetype::BIT);
    return bitOffset;
}

Handling::Handle<PlainMapping> FieldData::GetNestedObjectMapping () const noexcept
{
    EMERGENCE_ASSERT (archetype == FieldArchetype::NESTED_OBJECT);
    return nestedObjectMapping;
}

Handling::Handle<PlainMapping> FieldData::GetVectorItemMapping () const noexcept
{
    EMERGENCE_ASSERT (archetype == FieldArchetype::VECTOR);
    return vectorItemMapping;
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
      projected (_seed.projected),
      offset (_seed.offset),
      size (_seed.size),
      name (_seed.name)
{
    EMERGENCE_ASSERT (archetype != FieldArchetype::BIT);
    EMERGENCE_ASSERT (archetype != FieldArchetype::UNIQUE_STRING);
    EMERGENCE_ASSERT (archetype != FieldArchetype::NESTED_OBJECT);
}

FieldData::FieldData (FieldData::BitSeed _seed) noexcept
    : archetype (FieldArchetype::BIT),
      projected (_seed.projected),
      offset (_seed.offset),
      size (1u),
      name (_seed.name),
      bitOffset (_seed.bitOffset)
{
}

FieldData::FieldData (FieldData::UniqueStringSeed _seed) noexcept
    : archetype (FieldArchetype::UNIQUE_STRING),
      projected (_seed.projected),
      offset (_seed.offset),
      size (sizeof (Memory::UniqueString)),
      name (_seed.name)
{
}

FieldData::FieldData (FieldData::NestedObjectSeed _seed) noexcept
    : archetype (FieldArchetype::NESTED_OBJECT),
      projected (_seed.projected),
      offset (_seed.offset),
      name (_seed.name),
      nestedObjectMapping (std::move (_seed.nestedObjectMapping))
{
    EMERGENCE_ASSERT (nestedObjectMapping);
    size = nestedObjectMapping->GetObjectSize ();
}

FieldData::FieldData (FieldData::Utf8StringSeed _seed) noexcept
    : archetype (FieldArchetype::UTF8_STRING),
      projected (_seed.projected),
      offset (_seed.offset),
      size (sizeof (Container::Utf8String)),
      name (_seed.name)
{
}

FieldData::FieldData (FieldData::VectorSeed _seed) noexcept
    : archetype (FieldArchetype::VECTOR),
      projected (_seed.projected),
      offset (_seed.offset),
      name (_seed.name),
      vectorItemMapping (std::move (_seed.vectorItemMapping))
{
    EMERGENCE_ASSERT (vectorItemMapping);
    size = sizeof (Container::Vector<std::uint8_t>);
}

FieldData::FieldData (FieldData::PatchSeed _seed) noexcept
    : archetype (FieldArchetype::PATCH),
      projected (_seed.projected),
      offset (_seed.offset),
      size (sizeof (StandardLayout::Patch)),
      name (_seed.name)
{
}

FieldData::~FieldData ()
{
    if (archetype == FieldArchetype::NESTED_OBJECT)
    {
        nestedObjectMapping.~Handle ();
    }
    else if (archetype == FieldArchetype::VECTOR)
    {
        vectorItemMapping.~Handle ();
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
        if (currentField == owner->End ())
        {
            break;
        }

        UpdateCondition ();
    } while (!topConditionSatisfied);
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
      owner (const_cast<PlainMapping *> (_owner)),
      currentField (_field),
      nextCondition (_owner->firstCondition)
{
    EMERGENCE_ASSERT (currentField == owner->Begin () || currentField == owner->End ());
    // Technically condition can not start from the first field because there is no possible source fields.
    EMERGENCE_ASSERT (!_owner->firstCondition || _owner->firstCondition->sinceField > 0u);
}

void PlainMapping::ConditionalFieldIterator::UpdateCondition () noexcept
{
    const FieldId currentFieldId = owner->GetFieldId (*currentField);
    while (true)
    {
        if (topCondition && topCondition->untilField == currentFieldId)
        {
            topCondition = topCondition->popTo;
            // We've pushed popped condition onto stack, therefore previous top condition was satisfied.
            topConditionSatisfied = true;
            continue;
        }

        if (nextCondition && nextCondition->sinceField == currentFieldId)
        {
            EMERGENCE_ASSERT (nextCondition->popTo == topCondition);

            // If top condition is not satisfied, we're just moving until it is popped out.
            if (topConditionSatisfied)
            {
                topCondition = nextCondition;
                UpdateWhetherTopConditionSatisfied ();
            }

            nextCondition = nextCondition->next;
            continue;
        }

        break;
    }
}

void PlainMapping::ConditionalFieldIterator::UpdateWhetherTopConditionSatisfied () noexcept
{
    topConditionSatisfied = true;
    if (topCondition)
    {
        const FieldData *sourceField = owner->GetField (topCondition->sourceField);
        const auto *shifted = static_cast<const std::uint8_t *> (object) + sourceField->GetOffset ();

#define DO_OPERATION(Operation)                                                                                        \
    switch (sourceField->GetSize ())                                                                                   \
    {                                                                                                                  \
    case 1u:                                                                                                           \
        topConditionSatisfied = static_cast<std::uint64_t> (*shifted) Operation topCondition->argument;                \
        break;                                                                                                         \
                                                                                                                       \
    case 2u:                                                                                                           \
        topConditionSatisfied = static_cast<std::uint64_t> (*reinterpret_cast<const std::uint16_t *> (shifted))        \
                                    Operation topCondition->argument;                                                  \
        break;                                                                                                         \
                                                                                                                       \
    case 4u:                                                                                                           \
        topConditionSatisfied = static_cast<std::uint64_t> (*reinterpret_cast<const std::uint32_t *> (shifted))        \
                                    Operation topCondition->argument;                                                  \
        break;                                                                                                         \
                                                                                                                       \
    case 8u:                                                                                                           \
        topConditionSatisfied = *reinterpret_cast<const std::uint64_t *> (shifted) Operation topCondition->argument;   \
        break;                                                                                                         \
                                                                                                                       \
    default:                                                                                                           \
        EMERGENCE_ASSERT (false);                                                                                      \
        topConditionSatisfied = true;                                                                                  \
    }                                                                                                                  \
    break;

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

#undef DO_OPERATION
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

void PlainMapping::MoveConstruct (void *_address, void *_sourceAddress) const noexcept
{
    if (moveConstructor)
    {
        moveConstructor (_address, _sourceAddress);
    }
    else
    {
        EMERGENCE_ASSERT (false);
        Construct (_address);
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

const ConditionData *PlainMapping::GetFirstCondition () const noexcept
{
    return firstCondition;
}

FieldId PlainMapping::GetFieldId (const FieldData &_field) const
{
    EMERGENCE_ASSERT (&_field >= Begin ());
    EMERGENCE_ASSERT (&_field < End ());
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
    EMERGENCE_ASSERT (objectSize > 0u);
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
        ConditionData *next = condition->next;
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
    EMERGENCE_ASSERT (_newFieldCapacity >= fieldCount);
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
    EMERGENCE_ASSERT (!underConstruction);
    underConstruction = new (INITIAL_FIELD_CAPACITY) PlainMapping (_name, _objectSize, _objectAlignment);
    underConstruction->fieldCapacity = INITIAL_FIELD_CAPACITY;
    lastCondition = nullptr;
    topCondition = nullptr;
}

Handling::Handle<PlainMapping> PlainMappingBuilder::End () noexcept
{
    EMERGENCE_ASSERT (underConstruction);
    PlainMapping *finished = underConstruction->ChangeCapacity (underConstruction->fieldCount);
    underConstruction = nullptr;
    return finished;
}

void PlainMappingBuilder::SetConstructor (void (*_constructor) (void *)) noexcept
{
    EMERGENCE_ASSERT (underConstruction);
    underConstruction->constructor = _constructor;
}

void PlainMappingBuilder::SetMoveConstructor (void (*_constructor) (void *, void *)) noexcept
{
    EMERGENCE_ASSERT (underConstruction);
    underConstruction->moveConstructor = _constructor;
}

void PlainMappingBuilder::SetDestructor (void (*_destructor) (void *)) noexcept
{
    EMERGENCE_ASSERT (underConstruction);
    underConstruction->destructor = _destructor;
}

std::pair<FieldId, FieldData *> PlainMappingBuilder::AllocateField () noexcept
{
    EMERGENCE_ASSERT (underConstruction);
    EMERGENCE_ASSERT (underConstruction->fieldCount <= underConstruction->fieldCapacity);

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
    EMERGENCE_ASSERT (_sourceField < underConstruction->fieldCount);
    EMERGENCE_ASSERT (underConstruction->GetField (_sourceField)->archetype == FieldArchetype::UINT);

    auto *condition = new (ConditionData::GetPool ().Acquire ()) ConditionData {};
    condition->sinceField = underConstruction->fieldCount;
    condition->sourceField = _sourceField;
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
    EMERGENCE_ASSERT (topCondition);
    topCondition->untilField = underConstruction->fieldCount;
    topCondition = topCondition->popTo;
}
} // namespace Emergence::StandardLayout
