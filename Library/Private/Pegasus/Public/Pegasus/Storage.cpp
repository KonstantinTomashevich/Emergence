#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <cstring>

#include <API/Common/Implementation/Iterator.hpp>

#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
Storage::Allocator::Allocator (Storage::Allocator &&_other) noexcept : owner (_other.owner), current (_other.current)
{
    _other.owner = nullptr;
    _other.current = nullptr;
}

Storage::Allocator::~Allocator () noexcept
{
    if (owner)
    {
        if (current)
        {
            owner->InsertRecord (current);
        }

        owner->UnregisterWriter ();
    }
}

void *Storage::Allocator::Next () noexcept
{
    assert (owner);
    if (current)
    {
        owner->InsertRecord (current);
    }

    current = owner->AllocateRecord ();
    return current;
}

Storage::Allocator::Allocator (Storage *_owner) : owner (_owner)
{
    assert (owner);
    owner->RegisterWriter ();
}

using HashIndexIterator = Storage::HashIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (HashIndexIterator, iterator)

Handling::Handle<HashIndex> Storage::HashIndexIterator::operator* () const noexcept
{
    return iterator->index;
}

using OrderedIndexIterator = Storage::OrderedIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (OrderedIndexIterator, iterator)

Handling::Handle<OrderedIndex> Storage::OrderedIndexIterator::operator* () const noexcept
{
    return iterator->index;
}

using VolumetricIndexIterator = Storage::VolumetricIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (VolumetricIndexIterator, iterator)

Handling::Handle<VolumetricIndex> Storage::VolumetricIndexIterator::operator* () const noexcept
{
    return iterator->index;
}

using namespace Memory::Literals;

Storage::Storage (StandardLayout::Mapping _recordMapping) noexcept
    : records (Memory::Profiler::AllocationGroup {Memory::UniqueString {_recordMapping.GetName ()}},
               _recordMapping.GetObjectSize ()),
      hashIndexHeap (Memory::Profiler::AllocationGroup {records.GetAllocationGroup (), "HashIndex"_us}),
      orderedIndexHeap (Memory::Profiler::AllocationGroup {records.GetAllocationGroup (), "OrderedIndex"_us}),
      volumetricIndexHeap (Memory::Profiler::AllocationGroup {records.GetAllocationGroup (), "VolumetricIndex"_us}),
      recordMapping (std::move (_recordMapping))
{
    editedRecordBackup = records.Acquire ();
}

Storage::~Storage () noexcept
{
    assert (writers == 0u);
    assert (readers == 0u);

    // Destruct all existing records.

    if (!orderedIndices.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingEditCursor cursor =
            orderedIndices.Begin ()->index->LookupToEditAscending ({nullptr}, {nullptr});

        while (void *record = *cursor)
        {
            recordMapping.Destruct (record);
            ++cursor;
        }
    }
    else
    {
        for (void *record : records)
        {
            if (record != editedRecordBackup)
            {
                recordMapping.Destruct (record);
            }
        }
    }

    // Destruct all indices.

    for (auto &[index, mask] : hashIndices)
    {
        assert (index->CanBeDropped ());
        index->~HashIndex ();
        hashIndexHeap.Release (index, sizeof (HashIndex));
    }

    for (auto &[index, mask] : orderedIndices)
    {
        assert (index->CanBeDropped ());
        index->~OrderedIndex ();
        orderedIndexHeap.Release (index, sizeof (OrderedIndex));
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        assert (index->CanBeDropped ());
        index->~VolumetricIndex ();
        volumetricIndexHeap.Release (index, sizeof (VolumetricIndex));
    }
}

const StandardLayout::Mapping &Storage::GetRecordMapping () const noexcept
{
    return recordMapping;
}

Storage::Allocator Storage::AllocateAndInsert () noexcept
{
    return Allocator (this);
}

Handling::Handle<HashIndex> Storage::CreateHashIndex (
    const Container::Vector<StandardLayout::FieldId> &_indexedFields) noexcept
{
    assert (writers == 0u);
    assert (readers == 0u);
    constexpr std::size_t DEFAULT_INITIAL_BUCKETS = 32u;

    auto placeholder = hashIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<HashIndex> &holder = hashIndices.EmplaceBack (IndexHolder<HashIndex> {
        new (hashIndexHeap.Acquire (sizeof (HashIndex))) HashIndex (this, DEFAULT_INITIAL_BUCKETS, _indexedFields),
        0u});

    for (const StandardLayout::Field &indexedField : holder.index->GetIndexedFields ())
    {
        RegisterIndexedFieldUsage (indexedField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    if (!orderedIndices.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            orderedIndices.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

        while (const void *record = *cursor)
        {
            holder.index->InsertRecord (record);
            ++cursor;
        }
    }
    else
    {
        for (const void *record : records)
        {
            if (record != editedRecordBackup)
            {
                holder.index->InsertRecord (record);
            }
        }
    }

    return holder.index;
}

Handling::Handle<OrderedIndex> Storage::CreateOrderedIndex (StandardLayout::FieldId _indexedField) noexcept
{
    assert (writers == 0u);
    assert (readers == 0u);

    auto placeholder = orderedIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<OrderedIndex> &holder = orderedIndices.EmplaceBack (IndexHolder<OrderedIndex> {
        new (orderedIndexHeap.Acquire (sizeof (OrderedIndex))) OrderedIndex (this, _indexedField), 0u});

    RegisterIndexedFieldUsage (holder.index->GetIndexedField ());
    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    OrderedIndex::MassInsertionExecutor inserter = holder.index->StartMassInsertion ();

    if (orderedIndices.GetCount () > 1u)
    {
        // If there is another ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            orderedIndices.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

        while (const void *record = *cursor)
        {
            inserter.InsertRecord (record);
            ++cursor;
        }
    }
    else
    {
        for (const void *record : records)
        {
            if (record != editedRecordBackup)
            {
                inserter.InsertRecord (record);
            }
        }
    }

    return holder.index;
}

Handling::Handle<VolumetricIndex> Storage::CreateVolumetricIndex (
    const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept
{
    assert (writers == 0u);
    assert (readers == 0u);

    auto placeholder = volumetricIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<VolumetricIndex> &holder = volumetricIndices.EmplaceBack (IndexHolder<VolumetricIndex> {
        new (volumetricIndexHeap.Acquire (sizeof (VolumetricIndex))) VolumetricIndex (this, _dimensions), 0u});

    for (const VolumetricIndex::Dimension &dimension : holder.index->GetDimensions ())
    {
        RegisterIndexedFieldUsage (dimension.minBorderField);
        RegisterIndexedFieldUsage (dimension.maxBorderField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    if (!orderedIndices.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            orderedIndices.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

        while (const void *record = *cursor)
        {
            holder.index->InsertRecord (record);
            ++cursor;
        }
    }
    else
    {
        for (const void *record : records)
        {
            if (record != editedRecordBackup)
            {
                holder.index->InsertRecord (record);
            }
        }
    }
    return holder.index;
}

Storage::HashIndexIterator Storage::BeginHashIndices () const noexcept
{
    return Storage::HashIndexIterator (hashIndices.Begin ());
}

Storage::HashIndexIterator Storage::EndHashIndices () const noexcept
{
    return Storage::HashIndexIterator (hashIndices.End ());
}

Storage::OrderedIndexIterator Storage::BeginOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (orderedIndices.Begin ());
}

Storage::OrderedIndexIterator Storage::EndOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (orderedIndices.End ());
}

Storage::VolumetricIndexIterator Storage::BeginVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (volumetricIndices.Begin ());
}

Storage::VolumetricIndexIterator Storage::EndVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (volumetricIndices.End ());
}

void Storage::RegisterReader () noexcept
{
    // Writers counter can not be changed by thread safe operations, therefore it's ok to check it here.
    assert (writers == 0u);
    ++readers;
}

void Storage::RegisterWriter () noexcept
{
    assert (writers == 0u);
    assert (readers == 0u);
    ++writers;
}

void Storage::UnregisterReader () noexcept
{
    --readers;
}

void Storage::UnregisterWriter () noexcept
{
    assert (writers == 1u);
    --writers;

    for (auto &[index, mask] : hashIndices)
    {
        index->OnWriterClosed ();
    }

    for (auto &[index, mask] : orderedIndices)
    {
        index->OnWriterClosed ();
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        index->OnWriterClosed ();
    }
}

void *Storage::AllocateRecord () noexcept
{
    assert (readers == 0u);
    assert (writers == 1u);

    void *record = records.Acquire ();
    recordMapping.Construct (record);
    return record;
}

void Storage::InsertRecord (const void *_record) noexcept
{
    assert (_record);
    assert (readers == 0u);
    assert (writers == 1u);

    for (auto &[index, mask] : hashIndices)
    {
        index->InsertRecord (_record);
    }

    for (auto &[index, mask] : orderedIndices)
    {
        index->InsertRecord (_record);
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        index->InsertRecord (_record);
    }
}

void Storage::DeleteRecord (void *_record, const void *_requestedByIndex) noexcept
{
    assert (_record);
    assert (readers == 0u);
    assert (writers == 1u);

    for (auto &[index, mask] : hashIndices)
    {
        if (index != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    for (auto &[index, mask] : orderedIndices)
    {
        if (index != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        if (index != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    recordMapping.Destruct (_record);
    records.Release (_record);
}

void Storage::BeginRecordEdition (const void *_record) noexcept
{
    assert (_record);
    assert (editedRecordBackup);
    assert (readers == 0u);
    assert (writers == 1u);

    for (const IndexedField &indexedField : indexedFields)
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.field.GetArchetype ())
        {
        case StandardLayout::FieldArchetype::BIT:
        {
            uint8_t mask = 1u << indexedField.field.GetBitOffset ();
            if (*static_cast<const uint8_t *> (indexedField.field.GetValue (_record)) & mask)
            {
                *static_cast<uint8_t *> (indexedField.field.GetValue (editedRecordBackup)) |= mask;
            }
            else
            {
                *static_cast<uint8_t *> (indexedField.field.GetValue (editedRecordBackup)) &= ~mask;
            }

            break;
        }

        case StandardLayout::FieldArchetype::INT:
        case StandardLayout::FieldArchetype::UINT:
        case StandardLayout::FieldArchetype::FLOAT:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::UNIQUE_STRING:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        {
            memcpy (indexedField.field.GetValue (editedRecordBackup), indexedField.field.GetValue (_record),
                    indexedField.field.GetSize ());
            break;
        }

        case StandardLayout::FieldArchetype::STRING:
        {
            strncpy (static_cast<char *> (indexedField.field.GetValue (editedRecordBackup)),
                     static_cast<const char *> (indexedField.field.GetValue (_record)),
                     indexedField.field.GetSize () / sizeof (char));
            break;
        }
        }
    }
}

const void *Storage::GetEditedRecordBackup () const noexcept
{
    return editedRecordBackup;
}

bool Storage::EndRecordEdition (const void *_record, const void *_requestedByIndex) noexcept
{
    assert (_record);
    assert (editedRecordBackup);
    assert (readers == 0u);
    assert (writers == 1u);

    Constants::Storage::IndexedFieldMask changedIndexedFields = 0u;
    Constants::Storage::IndexedFieldMask fieldMask = 1u;

    for (const IndexedField &indexedField : indexedFields)
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreRecordValuesEqual (editedRecordBackup, _record, indexedField.field))
        {
            changedIndexedFields |= fieldMask;
        }

        fieldMask <<= 1u;
    }

    bool requesterAffected = false;
    for (auto &[index, mask] : hashIndices)
    {
        if (changedIndexedFields & mask)
        {
            if (index != _requestedByIndex)
            {
                index->OnRecordChanged (_record, editedRecordBackup);
            }
            else
            {
                requesterAffected = true;
            }
        }
    }

    for (auto &[index, mask] : orderedIndices)
    {
        if (changedIndexedFields & mask)
        {
            if (index != _requestedByIndex)
            {
                index->OnRecordChanged (_record, editedRecordBackup);
            }
            else
            {
                requesterAffected = true;
            }
        }
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        if (changedIndexedFields & mask)
        {
            if (index != _requestedByIndex)
            {
                index->OnRecordChanged (_record, editedRecordBackup);
            }
            else
            {
                requesterAffected = true;
            }
        }
    }

    return requesterAffected;
}

void Storage::DropIndex (const HashIndex &_index) noexcept
{
    for (const StandardLayout::Field &indexedField : _index.GetIndexedFields ())
    {
        UnregisterIndexedFieldUsage (indexedField);
    }

    hashIndices.EraseExchangingWithLast (std::find_if (hashIndices.Begin (), hashIndices.End (),
                                                       [&_index] (const IndexHolder<HashIndex> &_indexHolder) -> bool
                                                       {
                                                           return _indexHolder.index == &_index;
                                                       }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const OrderedIndex &_index) noexcept
{
    UnregisterIndexedFieldUsage (_index.GetIndexedField ());
    orderedIndices.EraseExchangingWithLast (
        std::find_if (orderedIndices.Begin (), orderedIndices.End (),
                      [&_index] (const IndexHolder<OrderedIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const VolumetricIndex &_index) noexcept
{
    for (const VolumetricIndex::Dimension &dimension : _index.GetDimensions ())
    {
        UnregisterIndexedFieldUsage (dimension.minBorderField);
        UnregisterIndexedFieldUsage (dimension.maxBorderField);
    }

    volumetricIndices.EraseExchangingWithLast (
        std::find_if (volumetricIndices.Begin (), volumetricIndices.End (),
                      [&_index] (const IndexHolder<VolumetricIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::RebuildIndexMasks () noexcept
{
    for (auto &[index, mask] : hashIndices)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }

    for (auto &[index, mask] : orderedIndices)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const HashIndex &_index) noexcept
{
    Constants::Storage::IndexedFieldMask indexMask = 0u;
    Constants::Storage::IndexedFieldMask currentFieldMask = 1u;

    for (const IndexedField &indexedField : indexedFields)
    {
        auto indexIterator = std::find_if (_index.GetIndexedFields ().Begin (), _index.GetIndexedFields ().End (),
                                           [&indexedField] (const StandardLayout::Field &_field) -> bool
                                           {
                                               return indexedField.field.IsSame (_field);
                                           });

        if (indexIterator != _index.GetIndexedFields ().End ())
        {
            indexMask |= currentFieldMask;
        }

        currentFieldMask <<= 1u;
    }

    return indexMask;
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const OrderedIndex &_index) noexcept
{
    auto fieldIterator = std::find_if (indexedFields.Begin (), indexedFields.End (),
                                       [&_index] (const IndexedField &_field) -> bool
                                       {
                                           return _field.field.IsSame (_index.GetIndexedField ());
                                       });

    assert (fieldIterator != indexedFields.End ());
    return 1u << (fieldIterator - indexedFields.Begin ());
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const VolumetricIndex &_index) noexcept
{
    Constants::Storage::IndexedFieldMask indexMask = 0u;

    for (const VolumetricIndex::Dimension &dimension : _index.GetDimensions ())
    {
        auto findField = [this] (const StandardLayout::Field &_field)
        {
            return std::find_if (indexedFields.Begin (), indexedFields.End (),
                                 [&_field] (const IndexedField &_indexedField) -> bool
                                 {
                                     return _indexedField.field.IsSame (_field);
                                 });
        };

        auto minIterator = findField (dimension.minBorderField);
        assert (minIterator != indexedFields.End ());
        indexMask |= 1u << (minIterator - indexedFields.Begin ());

        auto maxIterator = findField (dimension.maxBorderField);
        assert (maxIterator != indexedFields.End ());
        indexMask |= 1u << (maxIterator - indexedFields.Begin ());
    }

    return indexMask;
}

void Storage::RegisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept
{
    for (IndexedField &indexedField : indexedFields)
    {
        if (indexedField.field.IsSame (_field))
        {
            ++indexedField.usages;
            return;
        }
    }

    indexedFields.EmplaceBack (IndexedField {_field, 1u});
}

void Storage::UnregisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept
{
    auto iterator = std::find_if (indexedFields.Begin (), indexedFields.End (),
                                  [&_field] (const IndexedField &_indexedField) -> bool
                                  {
                                      return _indexedField.field.IsSame (_field);
                                  });

    assert (iterator != indexedFields.End ());
    assert (iterator->usages > 0u);
    --iterator->usages;

    if (iterator->usages == 0u)
    {
        indexedFields.EraseExchangingWithLast (iterator);
    }
}
} // namespace Emergence::Pegasus
