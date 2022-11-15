#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <API/Common/Implementation/Iterator.hpp>

#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>

namespace Emergence::Pegasus
{
Storage::Allocator::Allocator (Storage::Allocator &&_other) noexcept
    : owner (_other.owner),
      current (_other.current)
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
    EMERGENCE_ASSERT (owner);
    if (current)
    {
        owner->InsertRecord (current);
    }

    current = owner->AllocateRecord ();
    return current;
}

Storage::Allocator::Allocator (Storage *_owner)
    : owner (_owner)
{
    EMERGENCE_ASSERT (owner);
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

using SignalIndexIterator = Storage::SignalIndexIterator;

EMERGENCE_IMPLEMENT_BIDIRECTIONAL_ITERATOR_OPERATIONS_AS_WRAPPER (SignalIndexIterator, iterator)

Handling::Handle<SignalIndex> Storage::SignalIndexIterator::operator* () const noexcept
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
    : records (Memory::Profiler::AllocationGroup {"Records"_us},
               _recordMapping.GetObjectSize (),
               _recordMapping.GetObjectAlignment ()),
      hashIndexHeap (Memory::Profiler::AllocationGroup {"HashIndex"_us}),
      orderedIndexHeap (Memory::Profiler::AllocationGroup {"OrderedIndex"_us}),
      signalIndexHeap (Memory::Profiler::AllocationGroup {"SignalIndex"_us}),
      volumetricIndexHeap (Memory::Profiler::AllocationGroup {"VolumetricIndex"_us}),
      recordMapping (std::move (_recordMapping))
{
    editedRecordBackup = records.Acquire ();
}

Storage::~Storage () noexcept
{
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u);

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
        EMERGENCE_ASSERT (index->CanBeDropped ());
        index->~HashIndex ();
        hashIndexHeap.Release (index, sizeof (HashIndex));
    }

    for (auto &[index, mask] : orderedIndices)
    {
        EMERGENCE_ASSERT (index->CanBeDropped ());
        index->~OrderedIndex ();
        orderedIndexHeap.Release (index, sizeof (OrderedIndex));
    }

    for (auto &[index, mask] : signalIndices)
    {
        EMERGENCE_ASSERT (index->CanBeDropped ());
        index->~SignalIndex ();
        signalIndexHeap.Release (index, sizeof (SignalIndex));
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        EMERGENCE_ASSERT (index->CanBeDropped ());
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
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u);
    constexpr std::size_t DEFAULT_INITIAL_BUCKETS = 32u;

    auto placeholder = hashIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<HashIndex> &holder = hashIndices.EmplaceBack (
        IndexHolder<HashIndex> {new (hashIndexHeap.Acquire (sizeof (HashIndex), alignof (HashIndex)))
                                    HashIndex (this, DEFAULT_INITIAL_BUCKETS, _indexedFields),
                                0u});

    for (const StandardLayout::Field &indexedField : holder.index->GetIndexedFields ())
    {
        RegisterIndexedFieldUsage (indexedField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    InsertRecordsToIndex (holder.index);
    return holder.index;
}

Handling::Handle<OrderedIndex> Storage::CreateOrderedIndex (StandardLayout::FieldId _indexedField) noexcept
{
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u);

    auto placeholder = orderedIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<OrderedIndex> &holder = orderedIndices.EmplaceBack (
        IndexHolder<OrderedIndex> {new (orderedIndexHeap.Acquire (sizeof (OrderedIndex), alignof (OrderedIndex)))
                                       OrderedIndex (this, _indexedField),
                                   0u});

    RegisterIndexedFieldUsage (holder.index->GetIndexedField ());
    holder.indexedFieldMask = BuildIndexMask (*holder.index);

    // We use mass insertion to insert records into new index without overhead.
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

Handling::Handle<SignalIndex> Storage::CreateSignalIndex (
    StandardLayout::FieldId _indexedField, const std::array<uint8_t, sizeof (uint64_t)> &_signaledValue) noexcept
{
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u);

    auto placeholder = signalIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<SignalIndex> &holder = signalIndices.EmplaceBack (
        IndexHolder<SignalIndex> {new (signalIndexHeap.Acquire (sizeof (SignalIndex), alignof (SignalIndex)))
                                      SignalIndex (this, _indexedField, _signaledValue),
                                  0u});

    RegisterIndexedFieldUsage (holder.index->GetIndexedField ());
    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    InsertRecordsToIndex (holder.index);
    return holder.index;
}

Handling::Handle<VolumetricIndex> Storage::CreateVolumetricIndex (
    const Container::Vector<VolumetricIndex::DimensionDescriptor> &_dimensions) noexcept
{
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u);

    auto placeholder = volumetricIndexHeap.GetAllocationGroup ().PlaceOnTop ();
    IndexHolder<VolumetricIndex> &holder = volumetricIndices.EmplaceBack (IndexHolder<VolumetricIndex> {
        new (volumetricIndexHeap.Acquire (sizeof (VolumetricIndex), alignof (VolumetricIndex)))
            VolumetricIndex (this, _dimensions),
        0u});

    for (auto iterator = holder.index->BeginDimensions (); iterator != holder.index->EndDimensions (); ++iterator)
    {
        RegisterIndexedFieldUsage ((*iterator).minField);
        RegisterIndexedFieldUsage ((*iterator).maxField);
    }

    holder.indexedFieldMask = BuildIndexMask (*holder.index);
    InsertRecordsToIndex (holder.index);
    return holder.index;
}

Storage::HashIndexIterator Storage::BeginHashIndices () const noexcept
{
    return HashIndexIterator (hashIndices.Begin ());
}

Storage::HashIndexIterator Storage::EndHashIndices () const noexcept
{
    return HashIndexIterator (hashIndices.End ());
}

Storage::OrderedIndexIterator Storage::BeginOrderedIndices () const noexcept
{
    return OrderedIndexIterator (orderedIndices.Begin ());
}

Storage::OrderedIndexIterator Storage::EndOrderedIndices () const noexcept
{
    return OrderedIndexIterator (orderedIndices.End ());
}

SignalIndexIterator Storage::BeginSignalIndices () const noexcept
{
    return SignalIndexIterator (signalIndices.Begin ());
}

SignalIndexIterator Storage::EndSignalIndices () const noexcept
{
    return SignalIndexIterator (signalIndices.End ());
}

Storage::VolumetricIndexIterator Storage::BeginVolumetricIndices () const noexcept
{
    return VolumetricIndexIterator (volumetricIndices.Begin ());
}

Storage::VolumetricIndexIterator Storage::EndVolumetricIndices () const noexcept
{
    return VolumetricIndexIterator (volumetricIndices.End ());
}

void Storage::SetUnsafeReadAllowed (bool _allowed) noexcept
{
    // Unsafe access should be carefully controlled by user, therefore there should be no set-set or unset-unset calls.
    EMERGENCE_ASSERT (unsafeReadAllowed != _allowed);
    unsafeReadAllowed = _allowed;
}

void Storage::RegisterReader () noexcept
{
    // Writers counter can not be changed by thread safe operations, therefore it's ok to check it here.
    EMERGENCE_ASSERT (writers == 0u || unsafeReadAllowed);
    ++readers;
}

void Storage::RegisterWriter () noexcept
{
    EMERGENCE_ASSERT (writers == 0u);
    EMERGENCE_ASSERT (readers == 0u || unsafeReadAllowed);
    ++writers;
}

void Storage::UnregisterReader () noexcept
{
    --readers;
}

void Storage::UnregisterWriter () noexcept
{
    EMERGENCE_ASSERT (writers == 1u);
    --writers;

    VisitEveryIndex (
        [] (auto *_index, Constants::Storage::IndexedFieldMask /*unused*/)
        {
            _index->OnWriterClosed ();
        });
}

void *Storage::AllocateRecord () noexcept
{
    EMERGENCE_ASSERT (readers == 0u);
    EMERGENCE_ASSERT (writers == 1u);

    void *record = records.Acquire ();
    auto placeholder = records.GetAllocationGroup ().PlaceOnTop ();
    recordMapping.Construct (record);
    return record;
}

void Storage::InsertRecord (const void *_record) noexcept
{
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (readers == 0u);
    EMERGENCE_ASSERT (writers == 1u);

    VisitEveryIndex (
        [_record] (auto *_index, Constants::Storage::IndexedFieldMask /*unused*/)
        {
            _index->InsertRecord (_record);
        });
}

void Storage::DeleteRecord (void *_record, const void *_requestedByIndex) noexcept
{
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (readers == 0u);
    EMERGENCE_ASSERT (writers == 1u);

    VisitEveryIndex (
        [this, _record, _requestedByIndex] (auto *_index, Constants::Storage::IndexedFieldMask /*unused*/)
        {
            if (_index != _requestedByIndex)
            {
                _index->OnRecordDeleted (const_cast<const void *> (_record), editedRecordBackup);
            }
        });

    recordMapping.Destruct (_record);
    records.Release (_record);
}

void Storage::BeginRecordEdition (const void *_record) noexcept
{
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (editedRecordBackup);
    EMERGENCE_ASSERT (readers == 0u);
    EMERGENCE_ASSERT (writers == 1u);

    for (const IndexedField &indexedField : indexedFields)
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        EMERGENCE_ASSERT (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

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
    EMERGENCE_ASSERT (_record);
    EMERGENCE_ASSERT (editedRecordBackup);
    EMERGENCE_ASSERT (readers == 0u);
    EMERGENCE_ASSERT (writers == 1u);

    Constants::Storage::IndexedFieldMask changedIndexedFields = 0u;
    Constants::Storage::IndexedFieldMask fieldMask = 1u;

    for (const IndexedField &indexedField : indexedFields)
    {
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        EMERGENCE_ASSERT (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        if (!AreRecordValuesEqual (editedRecordBackup, _record, indexedField.field))
        {
            changedIndexedFields |= fieldMask;
        }

        fieldMask <<= 1u;
    }

    bool requesterAffected = false;
    VisitEveryIndex (
        [this, &requesterAffected, changedIndexedFields, _record, _requestedByIndex] (
            auto *_index, Constants::Storage::IndexedFieldMask _mask)
        {
            if (changedIndexedFields & _mask)
            {
                if (_index != _requestedByIndex)
                {
                    _index->OnRecordChanged (_record, editedRecordBackup);
                }
                else
                {
                    requesterAffected = true;
                }
            }
        });

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

void Storage::DropIndex (const SignalIndex &_index) noexcept
{
    UnregisterIndexedFieldUsage (_index.GetIndexedField ());
    signalIndices.EraseExchangingWithLast (
        std::find_if (signalIndices.Begin (), signalIndices.End (),
                      [&_index] (const IndexHolder<SignalIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const VolumetricIndex &_index) noexcept
{
    for (auto iterator = _index.BeginDimensions (); iterator != _index.EndDimensions (); ++iterator)
    {
        UnregisterIndexedFieldUsage ((*iterator).minField);
        UnregisterIndexedFieldUsage ((*iterator).maxField);
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
    VisitEveryIndex (
        [this] (auto *_index, Constants::Storage::IndexedFieldMask &_mask)
        {
            _mask = BuildIndexMask (*_index);
        });
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
#define BUILD_SINGLE_FIELD_MASK                                                                                        \
    auto fieldIterator = std::find_if (indexedFields.Begin (), indexedFields.End (),                                   \
                                       [&_index] (const IndexedField &_field) -> bool                                  \
                                       {                                                                               \
                                           return _field.field.IsSame (_index.GetIndexedField ());                     \
                                       });                                                                             \
                                                                                                                       \
    EMERGENCE_ASSERT (fieldIterator != indexedFields.End ());                                                          \
    return 1u << (fieldIterator - indexedFields.Begin ())
    BUILD_SINGLE_FIELD_MASK;
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const SignalIndex &_index) noexcept
{
    BUILD_SINGLE_FIELD_MASK;
#undef BUILD_SINGLE_FIELD_MASK
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const VolumetricIndex &_index) noexcept
{
    Constants::Storage::IndexedFieldMask indexMask = 0u;

    for (auto iterator = _index.BeginDimensions (); iterator != _index.EndDimensions (); ++iterator)
    {
        auto findField = [this] (const StandardLayout::Field &_field)
        {
            return std::find_if (indexedFields.Begin (), indexedFields.End (),
                                 [&_field] (const IndexedField &_indexedField) -> bool
                                 {
                                     return _indexedField.field.IsSame (_field);
                                 });
        };

        auto minIterator = findField ((*iterator).minField);
        EMERGENCE_ASSERT (minIterator != indexedFields.End ());
        indexMask |= 1u << (minIterator - indexedFields.Begin ());

        auto maxIterator = findField ((*iterator).maxField);
        EMERGENCE_ASSERT (maxIterator != indexedFields.End ());
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

    EMERGENCE_ASSERT (iterator != indexedFields.End ());
    EMERGENCE_ASSERT (iterator->usages > 0u);
    --iterator->usages;

    if (iterator->usages == 0u)
    {
        indexedFields.EraseExchangingWithLast (iterator);
    }
}

template <typename Index>
void Storage::InsertRecordsToIndex (Index *_index) noexcept
{
    if (!orderedIndices.Empty ())
    {
        // If there is an ordered index, fetch records from it, because it's faster than fetching them from pool.
        OrderedIndex::AscendingReadCursor cursor =
            orderedIndices.Begin ()->index->LookupToReadAscending ({nullptr}, {nullptr});

        while (const void *record = *cursor)
        {
            _index->InsertRecord (record);
            ++cursor;
        }
    }
    else
    {
        for (const void *record : records)
        {
            if (record != editedRecordBackup)
            {
                _index->InsertRecord (record);
            }
        }
    }
}

template <typename Functor>
void Storage::VisitEveryIndex (Functor _functor) noexcept
{
    for (auto &[index, mask] : hashIndices)
    {
        EMERGENCE_ASSERT (index);
        _functor (index, mask);
    }

    for (auto &[index, mask] : orderedIndices)
    {
        EMERGENCE_ASSERT (index);
        _functor (index, mask);
    }

    for (auto &[index, mask] : signalIndices)
    {
        EMERGENCE_ASSERT (index);
        _functor (index, mask);
    }

    for (auto &[index, mask] : volumetricIndices)
    {
        EMERGENCE_ASSERT (index);
        _functor (index, mask);
    }
}
} // namespace Emergence::Pegasus
