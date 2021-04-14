#pragma once

#include <cstdlib>
#include <iterator>
#include <type_traits>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
class FieldData final
{
public:
    FieldData (FieldArchetype _archetype, size_t _offset, size_t _size);

    FieldData (size_t _offset, uint_fast8_t _bitOffset);

    FieldData (size_t _offset, class PlainMapping *_instanceMapping);

    ~FieldData ();

    FieldArchetype GetArchetype () const;

    size_t GetOffset () const;

    size_t GetSize () const;

    uint_fast8_t GetBitOffset () const;

    class PlainMapping *GetInstanceMapping () const;

private:
    FieldArchetype archetype;
    std::size_t offset;
    std::size_t size;

    union
    {
        std::uint_fast8_t bitOffset;

        class PlainMapping *instanceMapping;
    };
};

class PlainMapping final
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

        explicit ConstIterator (const FieldData *_target) noexcept;

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
        const FieldData *target = nullptr;
    };

    void RegisterReference () noexcept;

    void UnregisterReference () noexcept;

    std::size_t GetObjectSize () const noexcept;

    std::size_t GetFieldCount () const noexcept;

    const FieldData *GetField (FieldId _field) const noexcept;

    FieldData *GetField (FieldId _field) noexcept;

    ConstIterator Begin () const noexcept;

    ConstIterator End () const noexcept;

    FieldId GetFieldId (const ConstIterator &_iterator) const;

private:
    friend class PlainMappingBuilder;

    explicit PlainMapping (std::size_t _objectSize) noexcept;

    ~PlainMapping () noexcept;

    std::size_t references = 0u;
    std::size_t objectSize = 0u;
    std::size_t fieldCount = 0u;

    // inplace_dynamic_array <FieldMeta> fields (fieldCount);
    // unused_memory additionalFieldPlaceholder (0u, infinity);
};

class PlainMappingBuilder
{
public:
    void Begin (std::size_t _objectSize) noexcept;

    PlainMapping *End () noexcept;

    FieldId AddField (FieldData _fieldData) noexcept;

private:
    static constexpr std::size_t INITIAL_FIELD_CAPACITY = 32u;

    void ReallocateMapping (std::size_t _fieldCapacity) noexcept;

    PlainMapping *underConstruction = nullptr;
    std::size_t fieldCapacity = 0u;
};
} // namespace Emergence::StandardLayout