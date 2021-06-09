#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include <Handling/HandleableBase.hpp>

#include <StandardLayout/Field.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

namespace Emergence::Pegasus
{
class HashIndex final : public Handling::HandleableBase
{
private:
    static constexpr std::size_t MAX_INDEXED_FIELDS = 8u;

public:
    // TODO: EditCursor must be increment before calling Storage::EndRecordEdition, because
    //       OnRecordChanged uses node extraction, which invalidates iterator to extracted element.

    const InplaceVector <StandardLayout::Field, MAX_INDEXED_FIELDS> &GetIndexedFields () const noexcept;

    bool CanBeDropped () const noexcept;

    void Drop () noexcept;

private:
    friend class Storage;

    /// To find changed record node hash must be calculated from backup, but comparator should use pointer comparison.
    struct RecordWithBackup final
    {
        const void *record;
        const void *backup;
    };

    struct Hasher final
    {
        using is_transparent = void;

        std::size_t operator () (const void *_record) const noexcept;

        std::size_t operator () (const RecordWithBackup &_record) const noexcept;

        HashIndex *owner;
    };

    struct Comparator final
    {
        using is_transparent = void;

        bool operator () (const void *_firstRecord, const void *_secondRecord) const noexcept;

        bool operator () (const void *_firstRecord, const RecordWithBackup &_recordWithBackup) const noexcept;

        HashIndex *owner;
    };

    explicit HashIndex (Storage *_owner, std::size_t _initialBuckets,
                        const std::vector <StandardLayout::Field> &_indexedFields);

    void InsertRecord (const void *_record) noexcept;

    void OnRecordChanged (const void *_record, const void *_recordBackup) noexcept;

    void OnWriterClosed () noexcept;

    Storage *owner;
    InplaceVector <StandardLayout::Field, MAX_INDEXED_FIELDS> indexedFields;
    std::unordered_multiset <const void *, Hasher, Comparator> records;
    std::vector <std::unordered_multiset <const void *, Hasher, Comparator>::node_type> changedNodes;
};
} // namespace Emergence::Pegasus