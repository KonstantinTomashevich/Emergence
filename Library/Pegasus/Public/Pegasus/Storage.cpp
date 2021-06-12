#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

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
    assert (owner);
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
    assert (owner);
    owner->RegisterWriter ();
}

Handling::Handle <HashIndex> Storage::HashIndexIterator::operator * () const noexcept
{
    return iterator->index.get ();
}

Storage::HashIndexIterator &Storage::HashIndexIterator::operator ++ () noexcept
{
    ++iterator;
    return *this;
}

Storage::HashIndexIterator Storage::HashIndexIterator::operator ++ (int) noexcept
{
    auto oldIterator = iterator;
    ++iterator;
    return Storage::HashIndexIterator (oldIterator);
}

Storage::HashIndexIterator &Storage::HashIndexIterator::operator -- () noexcept
{
    --iterator;
    return *this;
}

Storage::HashIndexIterator Storage::HashIndexIterator::operator -- (int) noexcept
{
    auto oldIterator = iterator;
    --iterator;
    return Storage::HashIndexIterator (oldIterator);
}

bool Storage::HashIndexIterator::operator == (const Storage::HashIndexIterator &_other) const noexcept
{
    return iterator == _other.iterator;
}

bool Storage::HashIndexIterator::operator != (const Storage::HashIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::HashIndexIterator::HashIndexIterator (HashIndexIterator::BaseIterator _iterator) noexcept
    : iterator (std::move (_iterator))
{
}

Handling::Handle <OrderedIndex> Storage::OrderedIndexIterator::operator * () const noexcept
{
    return iterator->index.get ();
}

Storage::OrderedIndexIterator &Storage::OrderedIndexIterator::operator ++ () noexcept
{
    ++iterator;
    return *this;
}

Storage::OrderedIndexIterator Storage::OrderedIndexIterator::operator ++ (int) noexcept
{
    auto oldIterator = iterator;
    ++iterator;
    return Storage::OrderedIndexIterator (oldIterator);
}

Storage::OrderedIndexIterator &Storage::OrderedIndexIterator::operator -- () noexcept
{
    --iterator;
    return *this;
}

Storage::OrderedIndexIterator Storage::OrderedIndexIterator::operator -- (int) noexcept
{
    auto oldIterator = iterator;
    --iterator;
    return Storage::OrderedIndexIterator (oldIterator);
}

bool Storage::OrderedIndexIterator::operator == (const Storage::OrderedIndexIterator &_other) const noexcept
{
    return iterator == _other.iterator;
}

bool Storage::OrderedIndexIterator::operator != (const Storage::OrderedIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::OrderedIndexIterator::OrderedIndexIterator (OrderedIndexIterator::BaseIterator _iterator) noexcept
    : iterator (std::move (_iterator))
{
}

Handling::Handle <VolumetricIndex> Storage::VolumetricIndexIterator::operator * () const noexcept
{
    return iterator->index.get ();
}

Storage::VolumetricIndexIterator &Storage::VolumetricIndexIterator::operator ++ () noexcept
{
    ++iterator;
    return *this;
}

Storage::VolumetricIndexIterator Storage::VolumetricIndexIterator::operator ++ (int) noexcept
{
    auto oldIterator = iterator;
    ++iterator;
    return Storage::VolumetricIndexIterator (oldIterator);
}

Storage::VolumetricIndexIterator &Storage::VolumetricIndexIterator::operator -- () noexcept
{
    --iterator;
    return *this;
}

Storage::VolumetricIndexIterator Storage::VolumetricIndexIterator::operator -- (int) noexcept
{
    auto oldIterator = iterator;
    --iterator;
    return Storage::VolumetricIndexIterator (oldIterator);
}

bool Storage::VolumetricIndexIterator::operator == (const Storage::VolumetricIndexIterator &_other) const noexcept
{
    return iterator == _other.iterator;
}

bool Storage::VolumetricIndexIterator::operator != (const Storage::VolumetricIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::VolumetricIndexIterator::VolumetricIndexIterator (VolumetricIndexIterator::BaseIterator _iterator) noexcept
    : iterator (std::move (_iterator))
{
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
    _other.indices.hash.Clear ();
    _other.indices.ordered.Clear ();
    _other.indices.volumetric.Clear ();
    _other.reflection.indexedFields.Clear ();

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

    // Assert that there are only self-references on indices.
#ifndef NDEBUG
    for (auto &[index, mask] : indices.hash)
    {
        assert (index->GetReferenceCount () == 1u);
    }

    for (auto &[index, mask] : indices.ordered)
    {
        assert (index->GetReferenceCount () == 1u);
    }

    for (auto &[index, mask] : indices.volumetric)
    {
        assert (index->GetReferenceCount () == 1u);
    }
#endif

    free (editedRecordBackup);
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
    // TODO: This method might be called from multiple threads. Remove this assert?
    assert (accessCounter.readers > 0u);
    --accessCounter.readers;
}

void Storage::UnregisterWriter () noexcept
{
    assert (accessCounter.writers == 1u);
    // TODO: Record edition routine must be closed. How to assert that?
    --accessCounter.writers;

    for (auto &[index, mask] : indices.hash)
    {
        index->OnWriterClosed ();
    }
//
//    for (auto &[index, mask] : indices.ordered)
//    {
//        index->OnWriterClosed ();
//    }
//
//    for (auto &[index, mask] : indices.volumetric)
//    {
//        index->OnWriterClosed ();
//    }
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
//
//    for (auto &[index, mask] : indices.ordered)
//    {
//        index->InsertRecord (_record);
//    }
//
//    for (auto &[index, mask] : indices.volumetric)
//    {
//        index->InsertRecord (_record);
//    }
}

void Storage::DeleteRecord (const void *_record, const void *_requestedByIndex) noexcept
{
    assert (_record);
    assert (accessCounter.readers == 0u);
    assert (accessCounter.writers == 1u);

    for (auto &[index, mask] : indices.hash)
    {
        if (index.get() != _requestedByIndex)
        {
            index->OnRecordDeleted (_record, editedRecordBackup);
        }
    }
//
//    for (auto &[index, mask] : indices.ordered)
//    {
//        if (index.get() != _requestedByIndex)
//        {
//            index->OnRecordDeleted (_record, editedRecordBackup);
//        }
//    }
//
//    for (auto &[index, mask] : indices.volumetric)
//    {
//        if (index.get() != _requestedByIndex)
//        {
//            index->OnRecordDeleted (_record, editedRecordBackup);
//        }
//    }
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
                if (*static_cast <const uint8_t *> (indexedField.field.GetValue (_record)) & mask)
                {
                    *static_cast <uint8_t *> (indexedField.field.GetValue (editedRecordBackup)) |= mask;
                }
                else
                {
                    *static_cast <uint8_t *> (indexedField.field.GetValue (editedRecordBackup)) &= ~mask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                memcpy (indexedField.field.GetValue (editedRecordBackup),
                        indexedField.field.GetValue (_record),
                        indexedField.field.GetSize ());
                break;
            }

            case StandardLayout::FieldArchetype::STRING:
            {
                strncpy (static_cast <char *> (indexedField.field.GetValue (editedRecordBackup)),
                         static_cast <const char *> (indexedField.field.GetValue (_record)),
                         indexedField.field.GetSize () / sizeof (char));
                break;
            }
        }
    }
}

void Storage::EndRecordEdition (const void *_record) noexcept
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

    for (auto &[index, mask] : indices.hash)
    {
        if (changedIndexedFields & mask)
        {
            index->OnRecordChanged (_record, editedRecordBackup);
        }
    }
//
//    for (auto &[index, mask] : indices.ordered)
//    {
//        if (changedIndexedFields & mask)
//        {
//            index->OnRecordChanged (record, editedRecordBackup);
//        }
//    }
//
//    for (auto &[index, mask] : indices.volumetric)
//    {
//        if (changedIndexedFields & mask)
//        {
//            index->OnRecordChanged (record, editedRecordBackup);
//        }
//    }
}

void Storage::DropIndex (const HashIndex &_index) noexcept
{
    for (const StandardLayout::Field &indexedField : _index.GetIndexedFields ())
    {
        UnregisterIndexedFieldUsage (indexedField);
    }

    indices.hash.EraseExchangingWithLast (
        std::find_if (indices.hash.Begin (), indices.hash.End (),
                      [&_index] (const IndexHolder <HashIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index.get () == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const OrderedIndex &_index) noexcept
{
    // TODO: Remove indexed fields.
    indices.ordered.EraseExchangingWithLast (
        std::find_if (indices.ordered.Begin (), indices.ordered.End (),
                      [&_index] (const IndexHolder <OrderedIndex> &_indexHolder) -> bool
                      {
                          return _indexHolder.index.get () == &_index;
                      }));

    RebuildIndexMasks ();
}

void Storage::DropIndex (const VolumetricIndex &_index) noexcept
{
    // TODO: Remove indexed fields.
    indices.volumetric.EraseExchangingWithLast (
        std::find_if (indices.volumetric.Begin (), indices.volumetric.End (),
                      [&_index] (const IndexHolder <VolumetricIndex> &_indexHolder) -> bool
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
        auto indexIterator = std::find_if (
            _index.GetIndexedFields ().Begin (), _index.GetIndexedFields ().End (),
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
    // TODO: Implement.
    // Suppress unused warning.
    _index.GetReferenceCount ();
    return 0u;
}

Constants::Storage::IndexedFieldMask Storage::BuildIndexMask (const VolumetricIndex &_index) noexcept
{
    // TODO: Implement.
    // Suppress unused warning.
    _index.GetReferenceCount ();
    return 0u;
}

void Storage::UnregisterIndexedFieldUsage (const StandardLayout::Field &_field) noexcept
{
    reflection.indexedFields.EraseExchangingWithLast (
        std::find_if (reflection.indexedFields.Begin (), reflection.indexedFields.End (),
                      [&_field] (const IndexedField &_indexedField) -> bool
                      {
                          return _indexedField.field.IsSame (_field);
                      }));
}
} // namespace Emergence::Pegasus