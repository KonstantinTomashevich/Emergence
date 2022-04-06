#pragma once

#include <array>
#include <atomic>
#include <memory>

#include <API/Common/Iterator.hpp>

#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Handling/Handle.hpp>

#include <Memory/OrderedPool.hpp>

#include <Pegasus/Constants/Storage.hpp>
#include <Pegasus/HashIndex.hpp>
#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/VolumetricIndex.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus
{
// TODO: Now all indices assume that OnRecordChanged and OnRecordDeleted can not be called for the same
//       record and that OnRecordChanged can not be called twice for one record during one edition cycle.

class Storage final
{
private:
    template <typename Index>
    struct IndexHolder
    {
        Index *index;
        Constants::Storage::IndexedFieldMask indexedFieldMask = 0u;
    };

public:
    template <typename Index>
    using IndexVector =
        Container::InplaceVector<Storage::IndexHolder<Index>, Constants::Storage::MAX_INDICES_OF_SAME_TYPE>;

    class Allocator final
    {
    public:
        Allocator (const Allocator &_other) = delete;

        Allocator (Allocator &&_other) noexcept;

        ~Allocator () noexcept;

        void *Next () noexcept;

        Allocator &operator= (const Allocator &_other) = delete;

        Allocator &operator= (Allocator &&_other) = delete;

    private:
        friend class Storage;

        explicit Allocator (Storage *_owner);

        Storage *owner;
        void *current = nullptr;
    };

    class HashIndexIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (HashIndexIterator, Handling::Handle<HashIndex>);

    private:
        friend class Storage;

        using BaseIterator = IndexVector<HashIndex>::ConstIterator;

        explicit HashIndexIterator (BaseIterator _baseIterator) noexcept;

        BaseIterator iterator;
    };

    class OrderedIndexIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (OrderedIndexIterator, Handling::Handle<OrderedIndex>);

    private:
        friend class Storage;

        using BaseIterator = IndexVector<OrderedIndex>::ConstIterator;

        explicit OrderedIndexIterator (BaseIterator _baseIterator) noexcept;

        BaseIterator iterator;
    };

    class VolumetricIndexIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (VolumetricIndexIterator, Handling::Handle<VolumetricIndex>);

    private:
        friend class Storage;

        using BaseIterator = IndexVector<VolumetricIndex>::ConstIterator;

        explicit VolumetricIndexIterator (BaseIterator _baseIterator) noexcept;

        BaseIterator iterator;
    };

    explicit Storage (StandardLayout::Mapping _recordMapping) noexcept;

    Storage (const Storage &_other) = delete;

    /// Move constructor could be useful, but we don't implement it
    /// because it's useless for RecordCollection implementation.
    Storage (Storage &&_other) = delete;

    ~Storage () noexcept;

    const StandardLayout::Mapping &GetRecordMapping () const noexcept;

    Allocator AllocateAndInsert () noexcept;

    Handling::Handle<HashIndex> CreateHashIndex (
        const Container::Vector<StandardLayout::FieldId> &_indexedFields) noexcept;

    Handling::Handle<OrderedIndex> CreateOrderedIndex (StandardLayout::FieldId _indexedField) noexcept;

    Handling::Handle<VolumetricIndex> CreateVolumetricIndex (
        const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept;

    HashIndexIterator BeginHashIndices () const noexcept;

    HashIndexIterator EndHashIndices () const noexcept;

    OrderedIndexIterator BeginOrderedIndices () const noexcept;

    OrderedIndexIterator EndOrderedIndices () const noexcept;

    VolumetricIndexIterator BeginVolumetricIndices () const noexcept;

    VolumetricIndexIterator EndVolumetricIndices () const noexcept;

    void SetUnsafeReadAllowed (bool _allowed) noexcept;

    Storage &operator= (const Storage &_other) = delete;

    /// Move assign could be useful, but we don't implement it
    /// because it's useless for RecordCollection implementation.
    Storage &operator= (Storage &&_other) noexcept = delete;

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
    /// \details Index, that requested deletion, usually already has iterator that points to requested
    ///          record and can do deletion faster. Therefore we identify this index by given
    ///          pointer and do not call OnRecordDeleted for this index.
    void DeleteRecord (void *_record, const void *_requestedByIndex) noexcept;

    void BeginRecordEdition (const void *_record) noexcept;

    /// \brief Ends record edition and informs indices about changed indexed fields.
    /// \details Index, that requested edition, usually already has iterator that points to edited
    //           record and can do deletion faster. Therefore we identify this index by given
    //           pointer and do not call OnRecordChanged for this index.
    // \return Are requester indexed fields changed?
    bool EndRecordEdition (const void *_record, const void *_requestedByIndex) noexcept;

    const void *GetEditedRecordBackup () const noexcept;

    void DropIndex (const HashIndex &_index) noexcept;

    void DropIndex (const OrderedIndex &_index) noexcept;

    void DropIndex (const VolumetricIndex &_index) noexcept;

    void RebuildIndexMasks () noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const HashIndex &_index) noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const OrderedIndex &_index) noexcept;

    Constants::Storage::IndexedFieldMask BuildIndexMask (const VolumetricIndex &_index) noexcept;

    void RegisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept;

    void UnregisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept;

    // TODO: Automatically shrink pool from time to time?
    Memory::OrderedPool records;

    Memory::Heap hashIndexHeap;
    Memory::Heap orderedIndexHeap;
    Memory::Heap volumetricIndexHeap;

    IndexVector<HashIndex> hashIndices;
    IndexVector<OrderedIndex> orderedIndices;
    IndexVector<VolumetricIndex> volumetricIndices;

    StandardLayout::Mapping recordMapping;
    Container::InplaceVector<IndexedField, Constants::Storage::MAX_INDEXED_FIELDS> indexedFields;

    std::atomic_size_t readers = 0u;
    std::size_t writers = 0u;

    static_assert (decltype (readers)::is_always_lock_free);

    void *editedRecordBackup = nullptr;

    bool unsafeReadAllowed = false;
};
} // namespace Emergence::Pegasus
