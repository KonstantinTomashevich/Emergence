#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

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
    return pointer->index.get ();
}

Storage::HashIndexIterator &Storage::HashIndexIterator::operator ++ () noexcept
{
    ++pointer;
    return *this;
}

Storage::HashIndexIterator Storage::HashIndexIterator::operator ++ (int) noexcept
{
    const IndexHolder <HashIndex> *oldPointer = pointer;
    ++pointer;
    return Storage::HashIndexIterator (oldPointer);
}

Storage::HashIndexIterator &Storage::HashIndexIterator::operator -- () noexcept
{
    --pointer;
    return *this;
}

Storage::HashIndexIterator Storage::HashIndexIterator::operator -- (int) noexcept
{
    const IndexHolder <HashIndex> *oldPointer = pointer;
    --pointer;
    return Storage::HashIndexIterator (oldPointer);
}

bool Storage::HashIndexIterator::operator == (const Storage::HashIndexIterator &_other) const noexcept
{
    return pointer == _other.pointer;
}

bool Storage::HashIndexIterator::operator != (const Storage::HashIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::HashIndexIterator::HashIndexIterator (const Storage::IndexHolder <HashIndex> *_pointer) noexcept
    : pointer (_pointer)
{
}

Handling::Handle <OrderedIndex> Storage::OrderedIndexIterator::operator * () const noexcept
{
    return pointer->index.get ();
}

Storage::OrderedIndexIterator &Storage::OrderedIndexIterator::operator ++ () noexcept
{
    ++pointer;
    return *this;
}

Storage::OrderedIndexIterator Storage::OrderedIndexIterator::operator ++ (int) noexcept
{
    const IndexHolder <OrderedIndex> *oldPointer = pointer;
    ++pointer;
    return Storage::OrderedIndexIterator (oldPointer);
}

Storage::OrderedIndexIterator &Storage::OrderedIndexIterator::operator -- () noexcept
{
    --pointer;
    return *this;
}

Storage::OrderedIndexIterator Storage::OrderedIndexIterator::operator -- (int) noexcept
{
    const IndexHolder <OrderedIndex> *oldPointer = pointer;
    --pointer;
    return Storage::OrderedIndexIterator (oldPointer);
}

bool Storage::OrderedIndexIterator::operator == (const Storage::OrderedIndexIterator &_other) const noexcept
{
    return pointer == _other.pointer;
}

bool Storage::OrderedIndexIterator::operator != (const Storage::OrderedIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::OrderedIndexIterator::OrderedIndexIterator (const Storage::IndexHolder <OrderedIndex> *_pointer) noexcept
    : pointer (_pointer)
{
}

Handling::Handle <VolumetricIndex> Storage::VolumetricIndexIterator::operator * () const noexcept
{
    return pointer->index.get ();
}

Storage::VolumetricIndexIterator &Storage::VolumetricIndexIterator::operator ++ () noexcept
{
    ++pointer;
    return *this;
}

Storage::VolumetricIndexIterator Storage::VolumetricIndexIterator::operator ++ (int) noexcept
{
    const IndexHolder <VolumetricIndex> *oldPointer = pointer;
    ++pointer;
    return Storage::VolumetricIndexIterator (oldPointer);
}

Storage::VolumetricIndexIterator &Storage::VolumetricIndexIterator::operator -- () noexcept
{
    --pointer;
    return *this;
}

Storage::VolumetricIndexIterator Storage::VolumetricIndexIterator::operator -- (int) noexcept
{
    const IndexHolder <VolumetricIndex> *oldPointer = pointer;
    --pointer;
    return Storage::VolumetricIndexIterator (oldPointer);
}

bool Storage::VolumetricIndexIterator::operator == (const Storage::VolumetricIndexIterator &_other) const noexcept
{
    return pointer == _other.pointer;
}

bool Storage::VolumetricIndexIterator::operator != (const Storage::VolumetricIndexIterator &_other) const noexcept
{
    return !(*this == _other);
}

Storage::VolumetricIndexIterator::VolumetricIndexIterator (const Storage::IndexHolder <VolumetricIndex> *_pointer) noexcept
    : pointer (_pointer)
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
    _other.indices.hashCount = 0u;
    _other.indices.orderedCount = 0u;
    _other.indices.volumetricCount = 0u;
    _other.reflection.indexedFieldsCount = 0u;

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
    return Storage::HashIndexIterator (&*indices.hash.begin ());
}

Storage::HashIndexIterator Storage::EndHashIndices () const noexcept
{
    return Storage::HashIndexIterator (&*(indices.hash.begin () + indices.hashCount));
}

Storage::OrderedIndexIterator Storage::BeginOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (&*indices.ordered.begin ());
}

Storage::OrderedIndexIterator Storage::EndOrderedIndices () const noexcept
{
    return Storage::OrderedIndexIterator (&*(indices.ordered.begin () + indices.orderedCount));
}

Storage::VolumetricIndexIterator Storage::BeginVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (&*indices.volumetric.begin ());
}

Storage::VolumetricIndexIterator Storage::EndVolumetricIndices () const noexcept
{
    return Storage::VolumetricIndexIterator (&*(indices.volumetric.begin () + indices.volumetricCount));
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
}

void *Storage::AllocateRecord () noexcept
{
    return records.Acquire ();
}

void Storage::InsertRecord (const void *record) noexcept
{
    assert (record);
    // Suppress unused warning.
    record = nullptr;
//    for (auto &[index, mask] : indices.hash)
//    {
//        index->InsertRecord (record);
//    }
//
//    for (auto &[index, mask] : indices.ordered)
//    {
//        index->InsertRecord (record);
//    }
//
//    for (auto &[index, mask] : indices.volumetric)
//    {
//        index->InsertRecord (record);
//    }
}

void Storage::BeginRecordEdition (const void *record) noexcept
{
    // TODO: Possible optimization. If indexed fields coverage is high
    //       (more than 50% of object size), just copy full object instead?

    assert (record);
    assert (editedRecordBackup);

    auto iterator = reflection.indexedFields.begin ();
    auto end = reflection.indexedFields.begin () + reflection.indexedFieldsCount;

    while (iterator != end)
    {
        const IndexedField &indexedField = *iterator;

        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.field.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::BIT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                uint8_t mask = 1u << indexedField.field.GetBitOffset ();

                if (*(static_cast <const uint8_t *> (record) + offset) & mask)
                {
                    *(static_cast <uint8_t *> (editedRecordBackup) + offset) |= mask;
                }
                else
                {
                    *(static_cast <uint8_t *> (editedRecordBackup) + offset) &= ~mask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                memcpy (static_cast <uint8_t *> (editedRecordBackup) + offset,
                        static_cast <const uint8_t *> (record) + offset,
                        indexedField.field.GetSize ());
                break;
            }

            case StandardLayout::FieldArchetype::STRING:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                strncpy (static_cast <char *> (editedRecordBackup) + offset,
                         static_cast <const char *> (record) + offset,
                         indexedField.field.GetSize () / sizeof (char));
                break;
            }
        }

        ++iterator;
    }
}

void Storage::EndRecordEdition (const void *record) noexcept
{
    assert (record);
    assert (editedRecordBackup);

    IndexedFieldMask changedIndexedFields = 0u;
    IndexedFieldMask fieldMask = 1u;

    auto iterator = reflection.indexedFields.begin ();
    auto end = reflection.indexedFields.begin () + reflection.indexedFieldsCount;

    while (iterator != end)
    {
        const IndexedField &indexedField = *iterator;
        // ::indexedFields should contain only leaf-fields, not intermediate nested objects.
        assert (indexedField.field.GetArchetype () != StandardLayout::FieldArchetype::NESTED_OBJECT);

        switch (indexedField.field.GetArchetype ())
        {
            case StandardLayout::FieldArchetype::BIT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                uint8_t mask = 1u << indexedField.field.GetBitOffset ();

                if ((*(static_cast <const uint8_t *> (record) + offset) & mask) !=
                    (*(static_cast <const uint8_t *> (editedRecordBackup) + offset) & mask))
                {
                    changedIndexedFields |= fieldMask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::INT:
            case StandardLayout::FieldArchetype::UINT:
            case StandardLayout::FieldArchetype::FLOAT:
            case StandardLayout::FieldArchetype::BLOCK:
            case StandardLayout::FieldArchetype::NESTED_OBJECT:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                if (memcmp (static_cast <const uint8_t *> (editedRecordBackup) + offset,
                            static_cast <const uint8_t *> (record) + offset,
                            indexedField.field.GetSize ()) != 0u)
                {
                    changedIndexedFields |= fieldMask;
                }

                break;
            }

            case StandardLayout::FieldArchetype::STRING:
            {
                std::size_t offset = indexedField.field.GetOffset ();
                if (strncmp (static_cast <const char *> (editedRecordBackup) + offset,
                             static_cast <const char *> (record) + offset,
                             indexedField.field.GetSize () / sizeof (char)) != 0u)
                {
                    changedIndexedFields |= fieldMask;
                }

                break;
            }
        }

        ++iterator;
        fieldMask <<= 1u;
    }

//    for (auto &[index, mask] : indices.hash)
//    {
//        if (changedIndexedFields & mask)
//        {
//            index->OnRecordChanged (record, editedRecordBackup);
//        }
//    }
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
} // namespace Emergence::Pegasus