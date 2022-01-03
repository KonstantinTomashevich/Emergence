#pragma once

#include <atomic>
#include <functional>
#include <unordered_set>
#include <vector>

#include <API/Common/Cursor.hpp>

#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Memory/Heap.hpp>

#include <Pegasus/Constants/HashIndex.hpp>
#include <Pegasus/IndexBase.hpp>

namespace Emergence::Pegasus
{
class HashIndex final : public IndexBase
{
public:
    using IndexedFieldVector =
        Container::InplaceVector<StandardLayout::Field, Constants::HashIndex::MAX_INDEXED_FIELDS>;

    struct LookupRequest final
    {
        /// \brief Pointer to an array with values for indexed field, used for lookup.
        /// \invariant Values must be stored one after another in the same order as ::indexedFields.
        /// \invariant Value size for fields with StandardLayout::FieldArchetype::STRING must always be equal to
        ///            StandardLayout::Field::GetSize, even if string length is less than this value.
        /// \invariant Values for fields with StandardLayout::FieldArchetype::BIT must passed as bytes in which all
        ///            bits should be zero's except bit with StandardLayout::Field::GetBitOffset.
        const void *indexedFieldValues = nullptr;
    };

    /// There is no sense to copy indices.
    HashIndex (const HashIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    HashIndex (HashIndex &&_other) = delete;

    ~HashIndex () = default;

    const IndexedFieldVector &GetIndexedFields () const noexcept;

    void Drop () noexcept;

    /// There is no sense to copy assign indices.
    HashIndex &operator= (const HashIndex &_other) = delete;

    /// Move assigning indices is forbidden, because otherwise user can move index out of Storage.
    HashIndex &operator= (HashIndex &&_other) = delete;

private:
    friend class Storage;

    /// \brief To find changed record node hash must be calculated
    ///        from backup, but comparator should use pointer comparison.
    struct RecordWithBackup final
    {
        const void *record = nullptr;
        const void *backup = nullptr;
    };

    struct Hasher final
    {
        using is_transparent = void;

        std::size_t operator() (const void *_record) const noexcept;

        std::size_t operator() (const RecordWithBackup &_record) const noexcept;

        std::size_t operator() (const LookupRequest &_request) const noexcept;

        HashIndex *owner;
    };

    struct Comparator final
    {
        using is_transparent = void;

        bool operator() (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator() (const void *_record, const RecordWithBackup &_recordWithBackup) const noexcept;

        bool operator() (const void *_record, const LookupRequest &_request) const noexcept;

        bool operator() (const LookupRequest &_request, const void *_record) const noexcept;

        HashIndex *owner;
    };

    using RecordHashSet = std::unordered_multiset<const void *, Hasher, Comparator, Memory::HeapSTD<const void *>>;

    explicit HashIndex (Storage *_owner,
                        std::size_t _initialBuckets,
                        const Container::Vector<StandardLayout::FieldId> &_indexedFields);

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    RecordHashSet::iterator DeleteRecordMyself (const RecordHashSet::iterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe (RecordHashSet::iterator _position) noexcept;

    void OnWriterClosed () noexcept;

    IndexedFieldVector indexedFields;
    RecordHashSet records;
    Container::Vector<RecordHashSet::node_type> changedNodes;

public:
    class ReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ReadCursor);

    private:
        friend class HashIndex;

        ReadCursor (HashIndex *_index,
                    RecordHashSet::const_iterator _begin,
                    RecordHashSet::const_iterator _end) noexcept;

        HashIndex *index;
        RecordHashSet::const_iterator current;
        RecordHashSet::const_iterator end;
    };

    class EditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (EditCursor);

    private:
        friend class HashIndex;

        EditCursor (HashIndex *_index, RecordHashSet::iterator _begin, RecordHashSet::iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        HashIndex *index;
        RecordHashSet::iterator current;
        RecordHashSet::iterator end;
    };

    ReadCursor LookupToRead (const LookupRequest &_request) noexcept;

    EditCursor LookupToEdit (const LookupRequest &_request) noexcept;
};
} // namespace Emergence::Pegasus
