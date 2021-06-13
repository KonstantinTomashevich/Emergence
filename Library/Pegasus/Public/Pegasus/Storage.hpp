#pragma once

#include <array>
#include <atomic>
#include <memory>

#include <Handling/Handle.hpp>

#include <Memory/Pool.hpp>

#include <Pegasus/Constants/Storage.hpp>
#include <Pegasus/HashIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/VolumetricIndex.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

namespace Emergence::Pegasus
{
class Storage final
{
private:
    template <typename Index>
    struct IndexHolder
    {
        std::unique_ptr <Index> index;
        Constants::Storage::IndexedFieldMask indexedFieldMask = 0u;
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

        using BaseIterator = InplaceVector <Storage::IndexHolder <HashIndex>,
                                            Constants::Storage::MAX_INDICES_OF_SAME_TYPE>::ConstIterator;

        explicit HashIndexIterator (BaseIterator _iterator) noexcept;

        BaseIterator iterator;
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

        using BaseIterator = InplaceVector <Storage::IndexHolder <OrderedIndex>,
                                            Constants::Storage::MAX_INDICES_OF_SAME_TYPE>::ConstIterator;

        explicit OrderedIndexIterator (BaseIterator _iterator) noexcept;

        BaseIterator iterator;
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

        using BaseIterator = InplaceVector <Storage::IndexHolder <VolumetricIndex>,
                                            Constants::Storage::MAX_INDICES_OF_SAME_TYPE>::ConstIterator;

        explicit VolumetricIndexIterator (BaseIterator _iterator) noexcept;

        BaseIterator iterator;
    };

    explicit Storage (StandardLayout::Mapping _recordMapping) noexcept;

    Storage (const Storage &_other) = delete;

    Storage (Storage &&_other) noexcept;

    ~Storage () noexcept;

    const StandardLayout::Mapping &GetRecordMapping () const noexcept;

    Allocator AllocateAndInsert () noexcept;

    Handling::Handle <HashIndex> CreateHashIndex (const std::vector <StandardLayout::FieldId> &_indexedFields) noexcept;

    HashIndexIterator BeginHashIndices () const noexcept;

    HashIndexIterator EndHashIndices () const noexcept;

    OrderedIndexIterator BeginOrderedIndices () const noexcept;

    OrderedIndexIterator EndOrderedIndices () const noexcept;

    VolumetricIndexIterator BeginVolumetricIndices () const noexcept;

    VolumetricIndexIterator EndVolumetricIndices () const noexcept;

private:
    friend class HashIndex;

    friend class OrderedIndex;

    friend class VolumetricIndex;

    struct IndexedField final
    {
        StandardLayout::Field field;
        std::size_t usages;
    };

    void RegisterReader () noexcept;

    void RegisterWriter () noexcept;

    void UnregisterReader () noexcept;

    void UnregisterWriter () noexcept;

    void *AllocateRecord () noexcept;

    void InsertRecord (const void *_record) noexcept;

    /// \brief Deletes record by request of given internal index.
    /// \details Index, that requested deletion, already has iterator that points to requested
    ///          record and can do deletion faster. Therefore we identify this index by given
    ///          pointer and do not call OnRecordDeleted for this index.
    void DeleteRecord (const void *_record, const void *_requestedByIndex) noexcept;

    void BeginRecordEdition (const void *_record) noexcept;

    void EndRecordEdition (const void *_record) noexcept;

    void DropIndex (const HashIndex &_index) noexcept;

    void DropIndex (const OrderedIndex &_index) noexcept;

    void DropIndex (const VolumetricIndex &_index) noexcept;

    void RebuildIndexMasks () noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const HashIndex &_index) noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const OrderedIndex &_index) noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const VolumetricIndex &_index) noexcept;

    void RegisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept;

    void UnregisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept;

    Memory::Pool records;

    struct
    {
        InplaceVector <IndexHolder <HashIndex>, Constants::Storage::MAX_INDICES_OF_SAME_TYPE> hash;
        InplaceVector <IndexHolder <OrderedIndex>, Constants::Storage::MAX_INDICES_OF_SAME_TYPE> ordered;
        InplaceVector <IndexHolder <VolumetricIndex>, Constants::Storage::MAX_INDICES_OF_SAME_TYPE> volumetric;
    } indices;

    struct
    {
        StandardLayout::Mapping recordMapping;
        InplaceVector <IndexedField, Constants::Storage::MAX_INDEXED_FIELDS> indexedFields;
    } reflection;

    struct
    {
        std::atomic_size_t readers = 0u;
        std::size_t writers = 0u;
    } accessCounter;

    void *editedRecordBackup = nullptr;
};
} // namespace Emergence::Pegasus