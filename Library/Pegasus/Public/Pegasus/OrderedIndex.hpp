#pragma once

#include <atomic>
#include <vector>

#include <Handling/HandleableBase.hpp>

#include <Pegasus/IndexBase.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::Pegasus
{
class OrderedIndex final : public IndexBase
{
public:
    struct Bound final
    {
        const void *boundValue;
    };

    class ReadCursor final
    {
    public:
        ReadCursor (const ReadCursor &_other) noexcept;

        ReadCursor (ReadCursor &&_other) noexcept;

        ~ReadCursor () noexcept;

        const void *operator * () const noexcept;

        ReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (const ReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (ReadCursor &&_other) = delete;

    private:
        friend class OrderedIndex;

        ReadCursor (OrderedIndex *_index,
                    std::vector <const void *>::const_iterator _begin,
                    std::vector <const void *>::const_iterator _end) noexcept;

        OrderedIndex *index;
        std::vector <const void *>::const_iterator current;
        std::vector <const void *>::const_iterator end;
    };

    class EditCursor final
    {
    public:
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other) noexcept;

        ~EditCursor () noexcept;

        void *operator * () noexcept;

        EditCursor &operator ~ ();

        EditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (const EditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (EditCursor &&_other) = delete;

    private:
        friend class OrderedIndex;

        EditCursor (OrderedIndex *_index,
                    std::vector <const void *>::iterator _begin,
                    std::vector <const void *>::iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        OrderedIndex *index;
        std::vector <const void *>::iterator current;
        std::vector <const void *>::iterator end;
    };

    class ReversedReadCursor final
    {
    public:
        ReversedReadCursor (const ReversedReadCursor &_other) noexcept;

        ReversedReadCursor (ReversedReadCursor &&_other) noexcept;

        ~ReversedReadCursor () noexcept;

        const void *operator * () const noexcept;

        ReversedReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReversedReadCursor &operator = (const ReversedReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReversedReadCursor &operator = (ReversedReadCursor &&_other) = delete;

    private:
        friend class OrderedIndex;

        ReversedReadCursor (OrderedIndex *_index,
                            std::vector <const void *>::const_reverse_iterator _begin,
                            std::vector <const void *>::const_reverse_iterator _end) noexcept;

        OrderedIndex *index;
        std::vector <const void *>::const_reverse_iterator current;
        std::vector <const void *>::const_reverse_iterator end;
    };

    class ReversedEditCursor final
    {
    public:
        ReversedEditCursor (const ReversedEditCursor &_other) = delete;

        ReversedEditCursor (ReversedEditCursor &&_other) noexcept;

        ~ReversedEditCursor () noexcept;

        void *operator * () noexcept;

        ReversedEditCursor &operator ~ ();

        ReversedEditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReversedEditCursor &operator = (const ReversedEditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReversedEditCursor &operator = (ReversedEditCursor &&_other) = delete;

    private:
        friend class OrderedIndex;

        ReversedEditCursor (OrderedIndex *_index,
                            std::vector <const void *>::reverse_iterator _begin,
                            std::vector <const void *>::reverse_iterator _end) noexcept;

        void BeginRecordEdition () const noexcept;

        OrderedIndex *index;
        std::vector <const void *>::reverse_iterator current;
        std::vector <const void *>::reverse_iterator end;
    };

    /// There is no sense to copy indices.
    OrderedIndex (const OrderedIndex &_other) = delete;

    /// Moving indices is forbidden, because otherwise user can move index out of Storage.
    OrderedIndex (OrderedIndex &&_other) = delete;

    ReadCursor LookupToRead (const Bound &_min, const Bound &_max) noexcept;

    ReversedReadCursor LookupToReadReversed (const Bound &_min, const Bound &_max) noexcept;

    EditCursor LookupToEdit (const Bound &_min, const Bound &_max) noexcept;

    ReversedEditCursor LookupToEditReversed (const Bound &_min, const Bound &_max) noexcept;

    StandardLayout::Field GetIndexedField () const noexcept;

    void Drop () noexcept;

private:
    friend class Storage;

    struct Comparator final
    {
        bool operator () (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator () (const Bound &_bound, const void *_record) const noexcept;

        bool operator () (const void *_record, const Bound &_bound) const noexcept;

        const OrderedIndex *owner;
    };

    struct InternalLookupResult
    {
        std::vector <const void *>::iterator begin;
        std::vector <const void *>::iterator end;
    };

    struct ChangedRecordInfo
    {
        std::size_t originalIndex;
        const void *record;
    };

    // TODO: There should be no more than one mass insertion executor at one moment of time. How to assert than?
    class MassInsertionExecutor final
    {
    public:
        ~MassInsertionExecutor () noexcept;

        void InsertRecord (const void *_record) noexcept;

    private:
        friend class OrderedIndex;

        explicit MassInsertionExecutor (OrderedIndex *_owner) noexcept;

        OrderedIndex *owner;
    };

    explicit OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField);

    InternalLookupResult InternalLookup (const Bound &_min, const Bound &_max) noexcept;

    std::vector <const void *>::const_iterator LocateRecord (
        const void *_record, const void *_recordBackup) const noexcept;

    void InsertRecord (const void *_record) noexcept;

    MassInsertionExecutor StartMassInsertion () noexcept;

    void OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept;

    void DeleteRecordMyself (const std::vector <const void *>::iterator &_position) noexcept;

    void DeleteRecordMyself (const std::vector <const void *>::reverse_iterator &_position) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnRecordChangedByMe (const std::vector <const void *>::iterator &_position) noexcept;

    void OnRecordChangedByMe (const std::vector <const void *>::reverse_iterator &_position) noexcept;

    void OnWriterClosed () noexcept;

    StandardLayout::Field indexedField;
    std::vector <const void *> records;
    std::vector <ChangedRecordInfo> changedRecords;
    std::vector <std::size_t> deletedRecordIndices;
};
} // namespace Emergence::Pegasus