#pragma once

#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Assert/Assert.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Memory/Heap.hpp>
#include <Memory/OrderedPool.hpp>
#include <Memory/UniqueString.hpp>

#include <StandardLayout/Field.hpp>
#include <StandardLayout/MappingBuilder.hpp>

namespace Emergence::StandardLayout
{
class PlainMapping;

class FieldData final
{
public:
    /// \brief Used to register fields with archetypes other than
    ///        FieldArchetype::BIT and FieldArchetype::NESTED_OBJECT.
    ///
    /// \details If FieldData::archetype is FieldArchetype::NESTED_OBJECT, FieldData must manage nestedObjectMapping as
    ///          counted reference. It's much easier to correctly implement this behaviour if FieldData construction
    ///          and destruction is done only in particular places (currently in PlainMappingBuilder and PlainMapping),
    ///          otherwise custom copy and move constructors will be required. Also, addition of custom move constructor
    ///          would violate realloc-movable requirement and create ambiguity.
    struct StandardSeed final
    {
        Memory::UniqueString name;

        FieldArchetype archetype = FieldArchetype::UINT;

        bool projected = false;

        std::size_t offset = 0u;

        std::size_t size = 0u;
    };

    /// \brief Used to register fields with FieldArchetype::BIT.
    struct BitSeed
    {
        Memory::UniqueString name;

        std::size_t offset = 0u;

        uint_fast8_t bitOffset = 0u;

        bool projected = false;
    };

    /// \brief Used to register fields with FieldArchetype::UNIQUE_STRING.
    struct UniqueStringSeed
    {
        Memory::UniqueString name;

        std::size_t offset = 0u;

        bool projected = false;
    };

    /// \brief Used to register fields with FieldArchetype::NESTED_OBJECT.
    struct NestedObjectSeed
    {
        Memory::UniqueString name;

        std::size_t offset = 0u;

        Handling::Handle<PlainMapping> nestedObjectMapping;

        bool projected = false;
    };

    /// \brief Used to register fields with FieldArchetype::VECTOR.
    struct VectorSeed
    {
        Memory::UniqueString name;

        std::size_t offset = 0u;

        Handling::Handle<PlainMapping> vectorItemMapping;

        bool projected = false;
    };

    /// \brief Used to register fields with FieldArchetype::PATCH.
    struct PatchSeed
    {
        Memory::UniqueString name;

        std::size_t offset = 0u;

        bool projected = false;
    };

    /// FieldData could only be moved through reallocation of PlainMapping.
    FieldData (const FieldData &_other) = delete;

    FieldData (FieldData &&_other) = delete;

    [[nodiscard]] FieldArchetype GetArchetype () const noexcept;

    [[nodiscard]] bool IsProjected () const noexcept;

    [[nodiscard]] size_t GetOffset () const noexcept;

    [[nodiscard]] size_t GetSize () const noexcept;

    [[nodiscard]] uint_fast8_t GetBitOffset () const noexcept;

    [[nodiscard]] Handling::Handle<PlainMapping> GetNestedObjectMapping () const noexcept;

    [[nodiscard]] Handling::Handle<PlainMapping> GetVectorItemMapping () const noexcept;

    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (FieldData);

private:
    /// PlainMapping deletes FieldData's.
    friend class PlainMapping;

    /// PlainMappingBuilder constructs FieldData's.
    friend class PlainMappingBuilder;

    // Empty constructor, required for arrays.
    FieldData () noexcept;

    explicit FieldData (StandardSeed _seed) noexcept;

    explicit FieldData (BitSeed _seed) noexcept;

    explicit FieldData (UniqueStringSeed _seed) noexcept;

    explicit FieldData (NestedObjectSeed _seed) noexcept;

    explicit FieldData (VectorSeed _seed) noexcept;

    explicit FieldData (PatchSeed _seed) noexcept;

    ~FieldData ();

    FieldArchetype archetype {FieldArchetype::INT};
    bool projected = false;

    std::size_t offset {0u};
    std::size_t size {0u};
    Memory::UniqueString name;

    union
    {
        std::uint_fast8_t bitOffset;

        Handling::Handle<PlainMapping> nestedObjectMapping;

        Handling::Handle<PlainMapping> vectorItemMapping;
    };
};

struct ConditionData
{
    static Memory::OrderedPool &GetPool () noexcept;

    FieldId sinceField = 0u;
    FieldId untilField = 0u;

    FieldId sourceField = 0u;
    ConditionalOperation operation = ConditionalOperation::EQUAL;
    std::uint64_t argument = 0u;

    ConditionData *popTo = nullptr;
    ConditionData *next = nullptr;
};

class PlainMapping final : public Handling::HandleableBase
{
public:
    class ConditionalFieldIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (ConditionalFieldIterator, const FieldData *);

    private:
        friend class PlainMapping;

        ConditionalFieldIterator (const PlainMapping *_owner, const FieldData *_field, const void *_object) noexcept;

        void UpdateCondition () noexcept;

        void UpdateWhetherTopConditionSatisfied () noexcept;

        const void *object = nullptr;
        Handling::Handle<PlainMapping> owner;
        const FieldData *currentField = nullptr;
        ConditionData *topCondition = nullptr;
        ConditionData *nextCondition = nullptr;
        bool topConditionSatisfied = true;
    };

    /// PlainMapping is not designed to be copied.
    PlainMapping (const PlainMapping &_other) = delete;

    /// PlainMapping is designed to be moved only through reallocation.
    PlainMapping (PlainMapping &&_other) = delete;

    [[nodiscard]] std::size_t GetObjectSize () const noexcept;

    [[nodiscard]] std::size_t GetObjectAlignment () const noexcept;

    [[nodiscard]] std::size_t GetFieldCount () const noexcept;

    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    void Construct (void *_address) const noexcept;

    void Destruct (void *_address) const noexcept;

    [[nodiscard]] const FieldData *GetField (FieldId _field) const noexcept;

    [[nodiscard]] const FieldData *Begin () const noexcept;

    [[nodiscard]] const FieldData *End () const noexcept;

    [[nodiscard]] ConditionalFieldIterator BeginConditional (const void *_object) const noexcept;

    [[nodiscard]] ConditionalFieldIterator EndConditional () const noexcept;

    const ConditionData *GetFirstCondition () const noexcept;

    [[nodiscard]] FieldId GetFieldId (const FieldData &_field) const;

    EMERGENCE_DELETE_ASSIGNMENT (PlainMapping);

private:
    /// PlainMappingBuilder handles PlainMapping allocation and construction routine.
    friend class PlainMappingBuilder;

    /// Handle calls PlainMapping destructor.
    template <typename>
    friend class Handling::Handle;

    /// \return Heap, used for PlainMapping allocation.
    static Memory::Heap &GetHeap () noexcept;

    /// \return Size of mapping object, that can hold up to _fieldCapacity fields.
    static std::size_t CalculateMappingSize (std::size_t _fieldCapacity) noexcept;

    explicit PlainMapping (Memory::UniqueString _name, std::size_t _objectSize, std::size_t _objectAlignment) noexcept;

    ~PlainMapping () noexcept;

    /// \brief Allocates mapping object, that can hold up to _fieldCapacity fields.
    ///
    /// \details PlainMapping uses malloc-based allocation to support runtime capacity changes using ::ChangeCapacity.
    void *operator new (std::size_t /*unused*/, std::size_t _fieldCapacity) noexcept;

    /// \brief PlainMapping has custom allocation logic and therefore needs custom deallocator.
    void operator delete (void *_pointer) noexcept;

    /// \brief Changes mapping object size by changing its field capacity.
    /// \invariant ::fieldCount is less or equal to _newFieldCapacity.
    ///
    /// \warning This operation can cause reallocation of mapping object.
    ///          Always use returned pointer instead of old one.
    /// \return Pointer to new location of this mapping object.
    PlainMapping *ChangeCapacity (std::size_t _newFieldCapacity) noexcept;

    std::size_t objectSize = 0u;
    std::size_t objectAlignment = 0u;
    std::size_t fieldCount = 0u;
    std::size_t fieldCapacity = 0u;
    Memory::UniqueString name;

    void (*constructor) (void *) = nullptr;
    void (*destructor) (void *) = nullptr;

    ConditionData *firstCondition = nullptr;
    FieldData fields[0u];
};

const FieldData *begin (const PlainMapping &_mapping) noexcept;

const FieldData *end (const PlainMapping &_mapping) noexcept;

class PlainMappingBuilder
{
public:
    PlainMappingBuilder () noexcept = default;

    /// PlainMappingBuilder is not designed to be copied or moved.
    PlainMappingBuilder (const PlainMappingBuilder &_other) = delete;

    PlainMappingBuilder (PlainMappingBuilder &&_other) noexcept;

    ~PlainMappingBuilder ();

    void Begin (Memory::UniqueString _name, std::size_t _objectSize, std::size_t _objectAlignment) noexcept;

    Handling::Handle<PlainMapping> End () noexcept;

    void SetConstructor (void (*_constructor) (void *)) noexcept;

    void SetDestructor (void (*_destructor) (void *)) noexcept;

    template <typename Seed>
    FieldId AddField (Seed _seed) noexcept;

    void PushCondition (FieldId _sourceField, ConditionalOperation _operation, std::uint64_t _argument) noexcept;

    void PopCondition () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PlainMappingBuilder);

private:
    static constexpr std::size_t INITIAL_FIELD_CAPACITY = 32u;

    std::pair<FieldId, FieldData *> AllocateField () noexcept;

    PlainMapping *underConstruction = nullptr;

    ConditionData *lastCondition = nullptr;

    ConditionData *topCondition = nullptr;
};

template <typename Seed>
FieldId PlainMappingBuilder::AddField (Seed _seed) noexcept
{
    auto [fieldId, allocatedField] = AllocateField ();
    new (allocatedField) FieldData (_seed);
    EMERGENCE_ASSERT (allocatedField->GetOffset () + allocatedField->GetSize () <= underConstruction->objectSize);
    return fieldId;
}
} // namespace Emergence::StandardLayout
