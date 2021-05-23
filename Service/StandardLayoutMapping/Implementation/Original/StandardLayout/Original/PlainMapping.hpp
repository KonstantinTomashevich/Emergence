#pragma once

#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
class FieldData;

class PlainMapping final : public Handling::HandleableBase
{
public:
    class ConstIterator final
    {
    public:
        using iterator_category = std::random_access_iterator_tag;

        using value_type = const FieldData;

        using difference_type = ptrdiff_t;

        using pointer = value_type *;

        using reference = value_type &;

        ConstIterator () noexcept;

        const FieldData &operator * () const noexcept;

        const FieldData *operator -> () const noexcept;

        ConstIterator &operator ++ () noexcept;

        ConstIterator operator ++ (int) noexcept;

        ConstIterator &operator -- () noexcept;

        ConstIterator operator -- (int) noexcept;

        ConstIterator operator + (ptrdiff_t _steps) const noexcept;

        friend ConstIterator operator + (ptrdiff_t _steps, const ConstIterator &_iterator) noexcept;

        ConstIterator &operator += (ptrdiff_t _steps) noexcept;

        ConstIterator operator - (ptrdiff_t _steps) const noexcept;

        ConstIterator &operator -= (ptrdiff_t _steps) noexcept;

        const FieldData &operator [] (std::size_t _index) const noexcept;

        ptrdiff_t operator - (const ConstIterator &_other) const noexcept;

        bool operator == (const ConstIterator &_other) const noexcept;

        bool operator != (const ConstIterator &_other) const noexcept;

        bool operator < (const ConstIterator &_other) const noexcept;

        bool operator > (const ConstIterator &_other) const noexcept;

        bool operator <= (const ConstIterator &_other) const noexcept;

        bool operator >= (const ConstIterator &_other) const noexcept;

    private:
        friend class PlainMapping;

        explicit ConstIterator (const FieldData *_target) noexcept;

        const FieldData *target = nullptr;
    };

    std::size_t GetObjectSize () const noexcept;

    std::size_t GetFieldCount () const noexcept;

    const FieldData *GetField (FieldId _field) const noexcept;

    FieldData *GetField (FieldId _field) noexcept;

    ConstIterator Begin () const noexcept;

    ConstIterator End () const noexcept;

    FieldId GetFieldId (const ConstIterator &_iterator) const;

    FieldId GetFieldId (const FieldData &_field) const;

private:
    /// PlainMappingBuilder handles PlainMapping allocation and construction routine.
    friend class PlainMappingBuilder;

    /// Handle calls PlainMapping destructor.
    template <Handling::Handleable Type>
    friend class Handling::Handle;

    /// \return Size of mapping object, that can hold up to _fieldCapacity fields.
    static std::size_t CalculateMappingSize (std::size_t _fieldCapacity) noexcept;

    explicit PlainMapping (std::size_t _objectSize) noexcept;

    ~PlainMapping () noexcept;

    /// \brief Allocates mapping object, that can hold up to _fieldCapacity fields.
    ///
    /// \details PlainMapping uses malloc-based allocation to support runtime capacity changes using ::ChangeCapacity.
    void *operator new (std::size_t _byteCount, std::size_t _fieldCapacity) noexcept;

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

    // inplace_dynamic_array <FieldMeta> fields (fieldCount);
    // unused_memory additionalFieldPlaceholder (0u, infinity);
};

class FieldData final
{
public:
    /// \brief Used to register fields with archetypes other than
    ///        FieldArchetype::BIT and FieldArchetype::NESTED_OBJECT.
    ///
    /// \details If FieldData::archetype is FieldArchetype::NESTED_OBJECT, FieldData must manage nestedObjectMapping as
    //           counted reference. It's much easier to correctly implement this behaviour if FieldData construction
    //           and destruction is done only in particular places (currently in PlainMappingBuilder and PlainMapping),
    //           otherwise custom copy and move constructors will be required. Also, addition of custom move constructor
    //           would violate realloc-movable requirement and create ambiguity.
    struct StandardSeed final
    {
        FieldArchetype archetype;

        std::size_t offset;

        std::size_t size;
    };

    /// \brief Used to register fields with FieldArchetype::BIT.
    struct BitSeed
    {
        std::size_t offset;

        uint_fast8_t bitOffset;
    };

    /// \brief Used to register fields with FieldArchetype::NESTED_OBJECT.
    struct NestedObjectSeed
    {
        std::size_t offset;

        Handling::Handle <PlainMapping> nestedObjectMapping;
    };

    FieldArchetype GetArchetype () const;

    size_t GetOffset () const;

    size_t GetSize () const;

    uint_fast8_t GetBitOffset () const;

    Handling::Handle <PlainMapping> GetNestedObjectMapping () const;

private:
    /// PlainMapping deletes FieldData's.
    friend class PlainMapping;

    /// PlainMappingBuilder constructs FieldData's.
    friend class PlainMappingBuilder;

    explicit FieldData (StandardSeed _seed) noexcept;

    explicit FieldData (BitSeed _seed) noexcept;

    explicit FieldData (NestedObjectSeed _seed) noexcept;

    ~FieldData ();

    FieldArchetype archetype;
    std::size_t offset;
    std::size_t size;

    union
    {
        std::uint_fast8_t bitOffset;

        Handling::Handle <PlainMapping> nestedObjectMapping;
    };
};

PlainMapping::ConstIterator begin (const PlainMapping &mapping) noexcept;

PlainMapping::ConstIterator end (const PlainMapping &mapping) noexcept;

class PlainMappingBuilder
{
public:
    ~PlainMappingBuilder ();

    void Begin (std::size_t _objectSize) noexcept;

    Handling::Handle <PlainMapping> End () noexcept;

    FieldId AddField (FieldData::StandardSeed _seed) noexcept;

    FieldId AddField (FieldData::BitSeed _seed) noexcept;

    FieldId AddField (FieldData::NestedObjectSeed _seed) noexcept;

private:
    static constexpr std::size_t INITIAL_FIELD_CAPACITY = 32u;

    std::pair <FieldId, FieldData *> AllocateField () noexcept;

    void ReallocateMapping (std::size_t _fieldCapacity) noexcept;

    PlainMapping *underConstruction = nullptr;
    std::size_t fieldCapacity = 0u;
};
} // namespace Emergence::StandardLayout