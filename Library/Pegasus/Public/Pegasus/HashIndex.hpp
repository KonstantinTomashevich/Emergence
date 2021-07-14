#pragma once

#include <atomic>
#include <functional>
#include <unordered_set>
#include <vector>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Pegasus/Constants/HashIndex.hpp>
#include <Pegasus/IndexBase.hpp>

#include <StandardLayout/Field.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

namespace Emergence::Pegasus
{
class HashIndex final : public IndexBase
{
public:
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

    const InplaceVector <StandardLayout::Field, Constants::HashIndex::MAX_INDEXED_FIELDS> &
    GetIndexedFields () const noexcept;

    void Drop () noexcept;

    /// There is no sense to copy assign indices.
    HashIndex &operator = (const HashIndex &_other) = delete;

    /// Move assigning indices is forbidden, because otherwise user can move index out of Storage.
    HashIndex &operator = (HashIndex &&_other) = delete;

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

        std::size_t operator () (const void *_record) const noexcept;

        std::size_t operator () (const RecordWithBackup &_record) const noexcept;

        std::size_t operator () (const LookupRequest &_request) const noexcept;

        HashIndex *owner;
    };

    struct Comparator final
    {
        using is_transparent = void;

        bool operator () (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator () (const void *_record, const RecordWithBackup &_recordWithBackup) const noexcept;

        bool operator () (const void *_record, const LookupRequest &_request) const noexcept;

        bool operator () (const LookupRequest &_request, const void *_record) const noexcept;

        HashIndex *owner;
    };

    // TODO: Custom allocator for better performance?
    using RecordHashSet = std::unordered_multiset <const void *, Hasher, Comparator>;

    explicit HashIndex (Storage *_owner, std::size_t _initialBuckets,
                        const std::vector <StandardLayout::FieldId> &_indexedFields);

    void InsertRecord (const void *_record) noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    RecordHashSet::iterator DeleteRecordMyself (const RecordHashSet::iterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe (RecordHashSet::iterator _position) noexcept;

    void OnWriterClosed () noexcept;

    InplaceVector <StandardLayout::Field, Constants::HashIndex::MAX_INDEXED_FIELDS> indexedFields;
    RecordHashSet records;
    std::vector <RecordHashSet::node_type> changedNodes;

public:
    class ReadCursor final
    {
    public:
        ReadCursor (const ReadCursor &_other) noexcept;

        ReadCursor (ReadCursor &&_other) noexcept;

        ~ReadCursor () noexcept;

        /// \return Pointer to current record or nullptr if there is no more records.
        const void *operator * () const noexcept;

        /// \brief Moves cursor to next record.
        /// \invariant ::current != ::end.
        ReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (const ReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (ReadCursor &&_other) = delete;

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
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other) noexcept;

        ~EditCursor () noexcept;

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        ///
        /// \invariant Cursor should not point to ending.
        ///
        /// \warning Record type is unknown during compile time, therefore appropriate
        ///          destructor should be called before record deletion.
        EditCursor &operator ~ () noexcept;

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (const EditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (EditCursor &&_other) = delete;

    private:
        friend class HashIndex;

        EditCursor (HashIndex *_index,
                    RecordHashSet::iterator _begin,
                    RecordHashSet::iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        HashIndex *index;
        RecordHashSet::iterator current;
        RecordHashSet::iterator end;
    };

    ReadCursor LookupToRead (const LookupRequest &_request) noexcept;

    EditCursor LookupToEdit (const LookupRequest &_request) noexcept;
};
} // namespace Emergence::Pegasus