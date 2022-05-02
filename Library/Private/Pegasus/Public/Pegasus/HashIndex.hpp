#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/HashMultiSet.hpp>
#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Memory/Heap.hpp>

#include <Pegasus/Constants/HashIndex.hpp>
#include <Pegasus/IndexBase.hpp>

#include <SyntaxSugar/Union.hpp>

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

    const IndexedFieldVector &GetIndexedFields () const noexcept;

    void Drop () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (HashIndex);

private:
    friend class Storage;

    /// \brief To find changed record node hash must be calculated
    ///        from backup, but comparator should use pointer comparison.
    struct RecordWithBackup final
    {
        const void *record = nullptr;
        const void *backup = nullptr;
    };

    // We have 2 hashing strategies:
    //
    // - Direct hashing, as its name suggests, treats field value as hash function result. It is only applicable for
    //   indices on one field that is smaller or equal to `size_t`. This approach allows to minimize performance
    //   cost of hash index usage for such one-field indices.
    // - Generic hashing treats fields as sequences of bytes and applies Hashing service to these sequences. This
    //   approach works for any combinations of fields, but is very ineffective for one-field indices.
    //
    // Hashing strategy is selected during index creation. To minimize runtime polymorphism cost we use union+switch
    // approach on external calls. This makes code compiler-optimization friendly (in comparison with virtual) and
    // provides control over each branch and instruction (unlike variant, that has some unavoidable `if`s inside).

    struct DirectHasher final
    {
        using is_transparent = void;

        DirectHasher (HashIndex *_owner) noexcept;

        std::size_t operator() (const void *_record) const noexcept;

        std::size_t operator() (const RecordWithBackup &_record) const noexcept;

        std::size_t operator() (const LookupRequest &_request) const noexcept;

        size_t mask = 0u;
        size_t offset = 0u;
    };

    struct DirectComparator final
    {
        using is_transparent = void;

        DirectComparator (HashIndex *_owner) noexcept;

        bool operator() (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator() (const void *_record, const RecordWithBackup &_recordWithBackup) const noexcept;

        bool operator() (const void *_record, const LookupRequest &_request) const noexcept;

        bool operator() (const LookupRequest &_request, const void *_record) const noexcept;

        size_t mask = 0u;
        size_t offset = 0u;
    };

    using DirectHashSet = Container::HashMultiSet<const void *, DirectHasher, DirectComparator>;

    struct GenericHasher final
    {
        using is_transparent = void;

        std::size_t operator() (const void *_record) const noexcept;

        std::size_t operator() (const RecordWithBackup &_record) const noexcept;

        std::size_t operator() (const LookupRequest &_request) const noexcept;

        HashIndex *owner;
    };

    struct GenericComparator final
    {
        using is_transparent = void;

        bool operator() (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator() (const void *_record, const RecordWithBackup &_recordWithBackup) const noexcept;

        bool operator() (const void *_record, const LookupRequest &_request) const noexcept;

        bool operator() (const LookupRequest &_request, const void *_record) const noexcept;

        HashIndex *owner;
    };

    using GenericHashSet = Container::HashMultiSet<const void *, GenericHasher, GenericComparator>;

    EMERGENCE_UNION2 (RecordHashSet, DirectHashSet, direct, GenericHashSet, generic);

    static_assert (std::is_same_v<DirectHashSet::const_iterator, GenericHashSet::const_iterator>);
    using RecordHashSetConstIterator = DirectHashSet::const_iterator;

    static_assert (std::is_same_v<DirectHashSet::iterator, GenericHashSet::iterator>);
    using RecordHashSetIterator = DirectHashSet::iterator;

    static_assert (std::is_same_v<DirectHashSet::node_type, GenericHashSet::node_type>);
    using RecordHashSetNode = DirectHashSet::node_type;

    explicit HashIndex (Storage *_owner,
                        std::size_t _initialBuckets,
                        const Container::Vector<StandardLayout::FieldId> &_indexedFields);

    ~HashIndex () noexcept;

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    RecordHashSetIterator DeleteRecordMyself0 (const RecordHashSetIterator &_position) noexcept;

    RecordHashSetIterator DeleteRecordMyself1 (const RecordHashSetIterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe0 (RecordHashSetIterator _position) noexcept;

    void OnRecordChangedByMe1 (RecordHashSetIterator _position) noexcept;

    void OnWriterClosed () noexcept;

    IndexedFieldVector indexedFields;
    size_t implementationSwitch = 0u;
    RecordHashSet records;
    Container::Vector<RecordHashSetNode> changedNodes;

public:
    class ReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ReadCursor);

    private:
        friend class HashIndex;

        ReadCursor (HashIndex *_index, RecordHashSetConstIterator _begin, RecordHashSetConstIterator _end) noexcept;

        HashIndex *index;
        RecordHashSetConstIterator current;
        RecordHashSetConstIterator end;
    };

    class EditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (EditCursor);

    private:
        friend class HashIndex;

        EditCursor (HashIndex *_index, RecordHashSetConstIterator _begin, RecordHashSetConstIterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        HashIndex *index;
        RecordHashSetConstIterator current;
        RecordHashSetConstIterator end;
    };

    ReadCursor LookupToRead (const LookupRequest &_request) noexcept;

    EditCursor LookupToEdit (const LookupRequest &_request) noexcept;
};
} // namespace Emergence::Pegasus
