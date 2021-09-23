#pragma once

#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Field.hpp>

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
        const char *name = nullptr;

        FieldArchetype archetype = FieldArchetype::UINT;

        std::size_t offset = 0u;

        std::size_t size = 0u;
    };

    /// \brief Used to register fields with FieldArchetype::BIT.
    struct BitSeed
    {
        const char *name = nullptr;

        std::size_t offset = 0u;

        uint_fast8_t bitOffset = 0u;
    };

    /// \brief Used to register fields with FieldArchetype::NESTED_OBJECT.
    struct NestedObjectSeed
    {
        const char *name = nullptr;

        std::size_t offset = 0u;

        Handling::Handle<PlainMapping> nestedObjectMapping;
    };

    /// FieldData could only be moved through reallocation of PlainMapping.
    FieldData (const FieldData &_other) = delete;

    FieldData (FieldData &&_other) = delete;

    FieldArchetype GetArchetype () const noexcept;

    size_t GetOffset () const noexcept;

    size_t GetSize () const noexcept;

    uint_fast8_t GetBitOffset () const noexcept;

    Handling::Handle<PlainMapping> GetNestedObjectMapping () const noexcept;

    const char *GetName () const noexcept;

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

    explicit FieldData (NestedObjectSeed _seed) noexcept;

    ~FieldData ();

    void CopyName (const char *_name) noexcept;

    FieldArchetype archetype;
    std::size_t offset;
    std::size_t size;

    union
    {
        std::uint_fast8_t bitOffset;

        Handling::Handle<PlainMapping> nestedObjectMapping;
    };

    /// \details Field name should not be inlined into FieldData object, because it would decrease field array cache
    ///          coherency: names are rarely accessed, but take a lot of space in comparison to frequently accessed
    ///          archetype, size and offset information.
    char *name;
};

class PlainMapping final : public Handling::HandleableBase
{
public:
    class ConstIterator final
    {
    public:
        ConstIterator () noexcept;

        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (ConstIterator, const FieldData &);

        const FieldData *operator-> () const noexcept;

    private:
        friend class PlainMapping;

        explicit ConstIterator (const FieldData *_target) noexcept;

        const FieldData *target = nullptr;
    };

    /// PlainMapping is not designed to be copied.
    PlainMapping (const PlainMapping &_other) = delete;

    /// PlainMapping is designed to be moved only through reallocation.
    PlainMapping (PlainMapping &&_other) = delete;

    std::size_t GetObjectSize () const noexcept;

    std::size_t GetFieldCount () const noexcept;

    const char *GetName () const noexcept;

    const FieldData *GetField (FieldId _field) const noexcept;

    FieldData *GetField (FieldId _field) noexcept;

    ConstIterator Begin () const noexcept;

    ConstIterator End () const noexcept;

    FieldId GetFieldId (const FieldData &_field) const;

    EMERGENCE_DELETE_ASSIGNMENT (PlainMapping);

private:
    /// PlainMappingBuilder handles PlainMapping allocation and construction routine.
    friend class PlainMappingBuilder;

    /// Handle calls PlainMapping destructor.
    template <typename>
    friend class Handling::Handle;

    /// \return Size of mapping object, that can hold up to _fieldCapacity fields.
    static std::size_t CalculateMappingSize (std::size_t _fieldCapacity) noexcept;

    explicit PlainMapping (const char *_name, std::size_t _objectSize) noexcept;

    ~PlainMapping () noexcept;

    /// \brief Allocates mapping object, that can hold up to _fieldCapacity fields.
    ///
    /// \details PlainMapping uses malloc-based allocation to support runtime capacity changes using ::ChangeCapacity.
    void *operator new (std::size_t, std::size_t _fieldCapacity) noexcept;

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
    std::size_t fieldCount = 0u;

    /// \details Mapping name should not be inlined into PlainMapping object, because it will
    ///          decrease cache coherency by adding huge chunk of rarely accessed data.
    char *name;

    FieldData fields[0u];
};

PlainMapping::ConstIterator begin (const PlainMapping &_mapping) noexcept;

PlainMapping::ConstIterator end (const PlainMapping &_mapping) noexcept;

class PlainMappingBuilder
{
public:
    PlainMappingBuilder () noexcept = default;

    /// PlainMappingBuilder is not designed to be copied or moved.
    PlainMappingBuilder (const PlainMappingBuilder &_other) = delete;

    PlainMappingBuilder (PlainMappingBuilder &&_other) = delete;

    ~PlainMappingBuilder ();

    void Begin (const char *_name, std::size_t _objectSize) noexcept;

    Handling::Handle<PlainMapping> End () noexcept;

    FieldId AddField (FieldData::StandardSeed _seed) noexcept;

    FieldId AddField (FieldData::BitSeed _seed) noexcept;

    FieldId AddField (FieldData::NestedObjectSeed _seed) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PlainMappingBuilder);

private:
    static constexpr std::size_t INITIAL_FIELD_CAPACITY = 32u;

    std::pair<FieldId, FieldData *> AllocateField () noexcept;

    void ReallocateMapping (std::size_t _fieldCapacity) noexcept;

    PlainMapping *underConstruction = nullptr;
    std::size_t fieldCapacity = 0u;
};
} // namespace Emergence::StandardLayout
