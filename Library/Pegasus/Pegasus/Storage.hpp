#pragma once

#include <array>
#include <atomic>
#include <memory>

#include <Handling/Handle.hpp>

#include <Memory/Pool.hpp>

#include <Pegasus/HashIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/VolumetricIndex.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus
{
class Storage final
{
private:
    static constexpr std::size_t MAX_INDEXED_FIELDS = 32u;

    using IndexedFieldMask = uint32_t;

    static_assert (sizeof (IndexedFieldMask) * 8u >= MAX_INDEXED_FIELDS);

    template <typename Index>
    struct IndexHolder
    {
        std::unique_ptr <Index> index;
        IndexedFieldMask indexedFieldMask = 0u;
    };

public:
    class Allocator final
    {
    public:
        Allocator (const Allocator &_other) = delete;

        Allocator (Allocator &&_other) noexcept;

        ~Allocator () noexcept;

        void *Next () noexcept;

        Allocator &operator = (const Allocator &_other) = delete;

        Allocator &operator = (Allocator &&_other) = delete;

    private:
        friend class Storage;

        explicit Allocator (Storage *_owner);

        Storage *owner;
        void *current = nullptr;
    };

    class HashIndexIterator final
    {
    public:
        Handling::Handle <HashIndex> operator * () const noexcept;

        HashIndexIterator &operator ++ () noexcept;

        HashIndexIterator operator ++ (int) noexcept;

        HashIndexIterator &operator -- () noexcept;

        HashIndexIterator operator -- (int) noexcept;

        bool operator == (const HashIndexIterator &_other) const noexcept;

        bool operator != (const HashIndexIterator &_other) const noexcept;

    private:
        friend class Storage;

        explicit HashIndexIterator (const Storage::IndexHolder<HashIndex> *_pointer) noexcept;

        const Storage::IndexHolder<HashIndex> *pointer;
    };

    class OrderedIndexIterator final
    {
    public:
        Handling::Handle <OrderedIndex> operator * () const noexcept;

        OrderedIndexIterator &operator ++ () noexcept;

        OrderedIndexIterator operator ++ (int) noexcept;

        OrderedIndexIterator &operator -- () noexcept;

        OrderedIndexIterator operator -- (int) noexcept;

        bool operator == (const OrderedIndexIterator &_other) const noexcept;

        bool operator != (const OrderedIndexIterator &_other) const noexcept;

    private:
        friend class Storage;

        explicit OrderedIndexIterator (const Storage::IndexHolder<OrderedIndex> *_pointer) noexcept;

        const Storage::IndexHolder<OrderedIndex> *pointer;
    };

    class VolumetricIndexIterator final
    {
    public:
        Handling::Handle <VolumetricIndex> operator * () const noexcept;

        VolumetricIndexIterator &operator ++ () noexcept;

        VolumetricIndexIterator operator ++ (int) noexcept;

        VolumetricIndexIterator &operator -- () noexcept;

        VolumetricIndexIterator operator -- (int) noexcept;

        bool operator == (const VolumetricIndexIterator &_other) const noexcept;

        bool operator != (const VolumetricIndexIterator &_other) const noexcept;

    private:
        friend class Storage;

        explicit VolumetricIndexIterator (const Storage::IndexHolder<VolumetricIndex> *_pointer) noexcept;

        const Storage::IndexHolder<VolumetricIndex> *pointer;
    };

    explicit Storage (StandardLayout::Mapping _recordMapping) noexcept;

    Storage (const Storage &_other) = delete;

    Storage (Storage &&_other) noexcept;

    ~Storage () noexcept;

    HashIndexIterator BeginHashIndices () const noexcept;

    HashIndexIterator EndHashIndices () const noexcept;

    OrderedIndexIterator BeginOrderedIndices () const noexcept;

    OrderedIndexIterator EndOrderedIndices () const noexcept;

    VolumetricIndexIterator BeginVolumetricIndices () const noexcept;

    VolumetricIndexIterator EndVolumetricIndices () const noexcept;

private:
    friend class Allocator;
    
    friend class HashIndexIterator;

    static constexpr std::size_t MAX_INDICES_OF_SAME_TYPE = 8u;

    void RegisterReader () noexcept;

    void RegisterWriter () noexcept;

    void UnregisterReader () noexcept;

    void UnregisterWriter () noexcept;

    void *AllocateRecord () noexcept;

    void InsertRecord (const void *record) noexcept;

    void BeginRecordEdition (const void *record) noexcept;

    void EndRecordEdition (const void *record) noexcept;

    struct IndexedField final
    {
        StandardLayout::Field field;
        std::size_t usages = 0u;
    };

    Memory::Pool records;

    struct
    {
        std::size_t hashCount = 0u;
        std::array <IndexHolder <HashIndex>, MAX_INDICES_OF_SAME_TYPE> hash;

        std::size_t orderedCount = 0u;
        std::array <IndexHolder <OrderedIndex>, MAX_INDICES_OF_SAME_TYPE> ordered;

        std::size_t volumetricCount = 0u;
        std::array <IndexHolder <VolumetricIndex>, MAX_INDICES_OF_SAME_TYPE> volumetric;
    } indices;

    struct
    {
        StandardLayout::Mapping recordMapping;
        std::size_t indexedFieldsCount = 0u;
        std::array <IndexedField, MAX_INDEXED_FIELDS> indexedFields;
    } reflection;

    struct
    {
        std::atomic_size_t readers = 0u;
        std::size_t writers = 0u;
    } accessCounter;

    void *editedRecordBackup = nullptr;
};
} // namespace Emergence::Pegasus