#pragma once

#include <atomic>

#include <API/Common/Cursor.hpp>

#include <Container/Vector.hpp>

#include <Handling/HandleableBase.hpp>

#include <Pegasus/IndexBase.hpp>

namespace Emergence::Pegasus
{
class OrderedIndex final : public IndexBase
{
public:
    struct Bound final
    {
        /// \brief Pointer to value, that has same type as ::indexedField, and will be used as lookup bound.
        const void *boundValue;
    };

    class AscendingReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (AscendingReadCursor);

    private:
        friend class OrderedIndex;

        AscendingReadCursor (OrderedIndex *_index,
                             Container::Vector<const void *>::const_iterator _begin,
                             Container::Vector<const void *>::const_iterator _end) noexcept;

        OrderedIndex *index;
        Container::Vector<const void *>::const_iterator current;
        Container::Vector<const void *>::const_iterator end;
    };

    class AscendingEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (AscendingEditCursor);

    private:
        friend class OrderedIndex;

        AscendingEditCursor (OrderedIndex *_index,
                             Container::Vector<const void *>::iterator _begin,
                             Container::Vector<const void *>::iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        OrderedIndex *index;
        Container::Vector<const void *>::iterator current;
        Container::Vector<const void *>::iterator end;
    };

    class DescendingReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (DescendingReadCursor);

    private:
        friend class OrderedIndex;

        DescendingReadCursor (OrderedIndex *_index,
                              Container::Vector<const void *>::const_reverse_iterator _begin,
                              Container::Vector<const void *>::const_reverse_iterator _end) noexcept;

        OrderedIndex *index;
        Container::Vector<const void *>::const_reverse_iterator current;
        Container::Vector<const void *>::const_reverse_iterator end;
    };

    class DescendingEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (DescendingEditCursor);

    private:
        friend class OrderedIndex;

        DescendingEditCursor (OrderedIndex *_index,
                              Container::Vector<const void *>::reverse_iterator _begin,
                              Container::Vector<const void *>::reverse_iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        OrderedIndex *index;
        Container::Vector<const void *>::reverse_iterator current;
        Container::Vector<const void *>::reverse_iterator end;
    };

    /// There is no sense to copy indices.
    OrderedIndex (const OrderedIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    OrderedIndex (OrderedIndex &&_other) = delete;

    ~OrderedIndex () = default;

    AscendingReadCursor LookupToReadAscending (const Bound &_min, const Bound &_max) noexcept;

    DescendingReadCursor LookupToReadDescending (const Bound &_min, const Bound &_max) noexcept;

    AscendingEditCursor LookupToEditAscending (const Bound &_min, const Bound &_max) noexcept;

    DescendingEditCursor LookupToEditDescending (const Bound &_min, const Bound &_max) noexcept;

    StandardLayout::Field GetIndexedField () const noexcept;

    void Drop () noexcept;

    /// There is no sense to copy assign indices.
    OrderedIndex &operator= (const OrderedIndex &_other) = delete;

    /// Move assigning indices is forbidden, because otherwise user can move index out of Storage.
    OrderedIndex &operator= (OrderedIndex &&_other) = delete;

private:
    friend class Storage;

    struct InternalLookupResult
    {
        Container::Vector<const void *>::iterator begin;
        Container::Vector<const void *>::iterator end;
    };

    struct ChangedRecordInfo
    {
        /// \warning Correct only if edition was done using cursor from this index, see ::hasEditCursor.
        std::size_t originalIndex;

        const void *record;
    };

    class MassInsertionExecutor final
    {
    public:
        MassInsertionExecutor (const MassInsertionExecutor &_other) = delete;

        MassInsertionExecutor (MassInsertionExecutor &&_other) = delete;

        ~MassInsertionExecutor () noexcept;

        void InsertRecord (const void *_record) noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (MassInsertionExecutor);

    private:
        friend class OrderedIndex;

        explicit MassInsertionExecutor (OrderedIndex *_owner) noexcept;

        OrderedIndex *owner;
    };

    explicit OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField);

    InternalLookupResult InternalLookup (const Bound &_min, const Bound &_max) noexcept;

    Container::Vector<const void *>::const_iterator LocateRecord (const void *_record,
                                                            const void *_recordBackup) const noexcept;

    void InsertRecord (const void *_record) noexcept;

    MassInsertionExecutor StartMassInsertion () noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void DeleteRecordMyself (const Container::Vector<const void *>::iterator &_position) noexcept;

    void DeleteRecordMyself (const Container::Vector<const void *>::reverse_iterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe (const Container::Vector<const void *>::iterator &_position) noexcept;

    void OnRecordChangedByMe (const Container::Vector<const void *>::reverse_iterator &_position) noexcept;

    void OnWriterClosed () noexcept;

    StandardLayout::Field indexedField;
    Container::Vector<const void *> records;

    /// \brief Debug-only, used to assert that there is always not more than one MassInsertionExecutor.
    bool massInsertionInProgress = false;

    /// If edition is done by cursor from this index, we should execute deletion and reinsertion after cursor is closed,
    /// because it allows to save time on ::records vector content shifts. It is safe, because edition by own cursor
    /// does not trigger any lookups.
    ///
    /// Edition by cursors from other indices will trigger lookup for each deletion or edition, therefore we need to
    /// keep ::records order correct and execute deletions right away. Edited records are deleted right away too, but
    /// reinserted only after edit cursor is closed. This behaviour allows us to use mass insertion optimization
    /// even if edition was done using cursor from other index.
    bool hasEditCursor = false;

    Container::Vector<ChangedRecordInfo> changedRecords;
    Container::Vector<std::size_t> deletedRecordIndices;
};
} // namespace Emergence::Pegasus
