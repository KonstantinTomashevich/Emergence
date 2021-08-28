#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

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

Storage::Allocator::Allocator (Storage *_owner) : owner (_owner), current (nullptr)
{
    assert (owner);
    owner->RegisterWriter ();
}

using HashIndexIterator = Storage::HashIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (HashIndexIterator, iterator)

Handling::Handle<HashIndex> Storage::HashIndexIterator::operator* () const noexcept
{
    return iterator->index.get ();
}

using OrderedIndexIterator = Storage::OrderedIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (OrderedIndexIterator, iterator)

Handling::Handle<OrderedIndex> Storage::OrderedIndexIterator::operator* () const noexcept
{
    return iterator->index.get ();
}

using VolumetricIndexIterator = Storage::VolumetricIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (VolumetricIndexIterator, iterator)

Handling::Handle<VolumetricIndex> Storage::VolumetricIndexIterator::operator* () const noexcept
{
    return iterator->index.get ();
}

Storage::Storage (StandardLayout::Mapping _recordMapping) noexcept
    : records (_recordMapping.GetObjectSize ()),
      reflection {.recordMapping = std::move (_recordMapping)}
{
    editedRecordBackup = malloc (reflection.recordMapping.GetObjectSize ());
}

Storage::Storage (Storage &&_other) noexcept
    : records (std::move (_other.records)),
      indices (std::move (_other.indices)),
      // Copy reflection, since we want source Storage to become valid empty storage for the same record type.
      reflection (_other.reflection),
      editedRecordBackup (_other.editedRecordBackup)
{
    // Update storage pointers in all moved indices.
    for (auto &[index, mask] : indices.hash)
    {
        index->storage = this;
    }

    for (auto &[index, mask] : indices.ordered)
    {
        index->storage = this;
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        index->storage = this;
    }

    // Allocate new record buffer for source storage. We capture its record buffer instead
    // of allocating new for us to preserve correct record edition routine state.
    _other.editedRecordBackup = malloc (reflection.recordMapping.GetObjectSize ());

    accessCounter.writers = _other.accessCounter.writers;
    _other.accessCounter.writers = 0u;

    // Move constructor does not declared thread safe, therefore we can copy value like that.
    std::size_t otherReaders = _other.accessCounter.readers;
    accessCounter.readers = otherReaders;
    _other.accessCounter.readers = 0u;
}

Storage::~Storage () noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);

    // Assert that all indices can be safely dropped.
#ifndef NDEBUG
    for (auto &[index, mask] : indices.hash)
    {
        assert (index->CanBeDropped ());
    }

    for (auto &[index, mask] : indices.ordered)
    {
        assert (index->CanBeDropped ());
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        assert (index->CanBeDropped ());
    }
#endif

    free (editedRecordBackup);
}

const StandardLayout::Mapping &Storage::GetRecordMapping () const noexcept
{
    return reflection.recordMapping;
}

Storage::Allocator Storage::AllocateAndInsert () noexcept
{
    return Allocator (this);
}

Handling::Handle<HashIndex> Storage::CreateHashIndex (
    const std::vector<StandardLayout::FieldId> &_indexedFields) noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);
    constexpr std::size_t defaultInitialBuckets = 32u;

    IndexHolder<HashIndex> &holder = indices.hash.EmplaceBack (IndexHolder<HashIndex> {
        std::unique_ptr<HashIndex> (new HashIndex (this, defaultInitialBuckets, _indexedFields)), 0u});

    for (const StandardLayout::Field &indexedField : holder.index->GetIndexedFields ())
    {
        RegisterIndexedFieldUsage (indexedField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    if (!indices.ordered.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            indices.ordered.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

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
            holder.index->InsertRecord (record);
        }
    }

    return holder.index.get ();
}

Handling::Handle<OrderedIndex> Storage::CreateOrderedIndex (StandardLayout::FieldId _indexedField) noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);

    IndexHolder<OrderedIndex> &holder = indices.ordered.EmplaceBack (
        IndexHolder<OrderedIndex> {std::unique_ptr<OrderedIndex> (new OrderedIndex (this, _indexedField)), 0u});

    RegisterIndexedFieldUsage (holder.index->GetIndexedField ());
    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    OrderedIndex::MassInsertionExecutor inserter = holder.index->StartMassInsertion ();

    if (indices.ordered.GetCount () > 1u)
    {
        // If there is another ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            indices.ordered.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

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
            inserter.InsertRecord (record);
        }
    }

    return holder.index.get ();
}

Handling::Handle<VolumetricIndex> Storage::CreateVolumetricIndex (
    const std::vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);

    IndexHolder<VolumetricIndex> &holder = indices.volumetric.EmplaceBack (
        IndexHolder<VolumetricIndex> {std::unique_ptr<VolumetricIndex> (new VolumetricIndex (this, _dimensions)), 0u});

    for (const VolumetricIndex::Dimension &dimension : holder.index->GetDimensions ())
    {
        RegisterIndexedFieldUsage (dimension.minBorderField);
        RegisterIndexedFieldUsage (dimension.maxBorderField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    if (!indices.ordered.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            indices.ordered.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

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
            holder.index->InsertRecord (record);
        }
    }

    return holder.index.get ();
}

Storage::HashIndexIterator Storage::BeginHashIndices () const noexcept
{
    return Storage::HashIndexIterator (indices.hash.Begin ());
}

Storage::HashIndexIterator Storage::EndHashIndices () const noexcept
{
    return Storage::HashIndexIterator (indices.hash.End ());
}

Storage::OrderedIndexIterator Storage::BeginOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (indices.ordered.Begin ());
}

Storage::OrderedIndexIterator Storage::EndOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (indices.ordered.End ());
}

Storage::VolumetricIndexIterator Storage::BeginVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (indices.volumetric.Begin ());
}

Storage::VolumetricIndexIterator Storage::EndVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (indices.volumetric.End ());
}

Storage &Storage::operator= (Storage &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Storage ();
        new (this) Storage (std::move (_other));
    }

    return *this;
}

void Storage::RegisterReader () noexcept
{
    // Writers counter can not be changed by thread safe operations, therefore it's ok to check it here.
    assert (accessCounter.writers == 0u);
    ++accessCounter.readers;
}

void Storage::RegisterWriter () noexcept
{
    assert (accessCounter.writers == 0u);
    assert (accessCounter.readers == 0u);
    ++accessCounter.writers;
}

void Storage::UnregisterReader () noexcept
{
    --accessCounter.readers;
}

void Storage::UnregisterWriter () noexcept
{
    assert (accessCounter.writers == 1u);
    --accessCounter.writers;

    for (auto &[index, mask] : indices.hash)
    {
        index->OnWriterClosed ();
    }

    for (auto &[index, mask] : indices.ordered)
    {
        index->OnWriterClosed ();
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        index->OnWriterClosed ();
    }
}

void *Storage::AllocateRecord () noexcept
{
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);
    return records.Acquire ();
}

void Storage::InsertRecord (const void *_record) noexcept
{
    assert (_record);
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);

    for (auto &[index, mask] : indices.hash)
    {
        index->InsertRecord (_record);
    }

    for (auto &[index, mask] : indices.ordered)
    {
        index->InsertRecord (_record);
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        index->InsertRecord (_record);
    }
}

void Storage::DeleteRecord (void *_record, const void *_requestedByIndex) noexcept
{
    assert (_record);
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);

    for (auto &[index, mask] : indices.hash)
    {
        if (index.get () != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    for (auto &[index, mask] : indices.ordered)
    {
        if (index.get () != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        if (index.get () != _requestedByIndex)
        {
            index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
        }
    }

    records.Release (_record);
}

void Storage::BeginRecordEdition (const void *_record) noexcept
{
    // TODO: Possible optimization. If indexed fields coverage is high
    //       (more than 50% of object size), just copy full object instead?

    assert (_record);
    assert (editedRecordBackup);
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);

    for (const IndexedField &indexedField : reflection.indexedFields)
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
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);

    Constants::Storage::IndexedFieldMask changedIndexedFields = 0u;
    Constants::Storage::IndexedFieldMask fieldMask = 1u;

    for (const IndexedField &indexedField : reflection.indexedFields)
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
    for (auto &[index, mask] : indices.hash)
    {
        if (changedIndexedFields & mask)
        {
            if (index.get () != _requestedByIndex)
            {
                index->OnRecordChanged (_record, editedRecordBackup);
            }
            else
            {
                requesterAffected = true;
            }
        }
    }

    for (auto &[index, mask] : indices.ordered)
    {
        if (changedIndexedFields & mask)
        {
            if (index.get () != _requestedByIndex)
            {
                index->OnRecordChanged (_record, editedRecordBackup);
            }
            else
            {
                requesterAffected = true;
            }
        }
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        if (changedIndexedFields & mask)
        {
            if (index.get () != _requestedByIndex)
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

    indices.hash.EraseExchangingWithLast (std::find_if (indices.hash.Begin (), indices.hash.End (),
                                                        [&_index] (const IndexHolder<HashIndex> &_indexHolder) -> bool
                                                        {
                                                            return _indexHolder.index.get () == &_index;
                                                        }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const OrderedIndex &_index) noexcept
{
    UnregisterIndexedFieldUsage (_index.GetIndexedField ());
    indices.ordered.EraseExchangingWithLast (
        std::find_if (indices.ordered.Begin (), indices.ordered.End (),
                      [&_index] (const IndexHolder<OrderedIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index.get () == &_index;
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

    indices.volumetric.EraseExchangingWithLast (
        std::find_if (indices.volumetric.Begin (), indices.volumetric.End (),
                      [&_index] (const IndexHolder<VolumetricIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index.get () == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::RebuildIndexMasks () noexcept
{
    for (auto &[index, mask] : indices.hash)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }

    for (auto &[index, mask] : indices.ordered)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        assert (index);
        mask = BuildIndexMask (*index);
    }
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const HashIndex &_index) noexcept
{
    Constants::Storage::IndexedFieldMask indexMask = 0u;
    Constants::Storage::IndexedFieldMask currentFieldMask = 1u;

    for (const IndexedField &indexedField : reflection.indexedFields)
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
    auto fieldIterator = std::find_if (reflection.indexedFields.Begin (), reflection.indexedFields.End (),
                                       [&_index] (const IndexedField &_field) -> bool
                                       {
                                           return _field.field.IsSame (_index.GetIndexedField ());
                                       });

    assert (fieldIterator != reflection.indexedFields.End ());
    return 1u << (fieldIterator - reflection.indexedFields.Begin ());
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const VolumetricIndex &_index) noexcept
{
    Constants::Storage::IndexedFieldMask indexMask = 0u;

    for (const VolumetricIndex::Dimension &dimension : _index.GetDimensions ())
    {
        auto findField = [this] (const StandardLayout::Field &_field)
        {
            return std::find_if (reflection.indexedFields.Begin (), reflection.indexedFields.End (),
                                 [&_field] (const IndexedField &_indexedField) -> bool
                                 {
                                     return _indexedField.field.IsSame (_field);
                                 });
        };

        auto minIterator = findField (dimension.minBorderField);
        assert (minIterator != reflection.indexedFields.End ());
        indexMask |= 1u << (minIterator - reflection.indexedFields.Begin ());

        auto maxIterator = findField (dimension.maxBorderField);
        assert (maxIterator != reflection.indexedFields.End ());
        indexMask |= 1u << (maxIterator - reflection.indexedFields.Begin ());
    }

    return indexMask;
}

void Storage::RegisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept
{
    for (IndexedField &indexedField : reflection.indexedFields)
    {
        if (indexedField.field.IsSame (_field))
        {
            ++indexedField.usages;
            return;
        }
    }

    reflection.indexedFields.EmplaceBack (IndexedField {_field, 1u});
}

void Storage::UnregisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept
{
    auto iterator = std::find_if (reflection.indexedFields.Begin (), reflection.indexedFields.End (),
                                  [&_field] (const IndexedField &_indexedField) -> bool
                                  {
                                      return _indexedField.field.IsSame (_field);
                                  });

    assert (iterator != reflection.indexedFields.End ());
    assert (iterator->usages > 0u);
    --iterator->usages;

    if (iterator->usages == 0u)
    {
        reflection.indexedFields.EraseExchangingWithLast (iterator);
    }
}
} // namespace Emergence::Pegasus
