#include <algorithm>
#include <cassert>

#include <Pegasus/OrderedIndex.hpp>
#include <Pegasus/RecordUtility.hpp>
#include <Pegasus/Storage.hpp>
#include <utility>

namespace Emergence::Pegasus
{
OrderedIndex::ReadCursor::ReadCursor (const OrderedIndex::ReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current),
      end (_other.end)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReadCursor::ReadCursor (OrderedIndex::ReadCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReadCursor::~ReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *OrderedIndex::ReadCursor::operator * () const noexcept
{
    assert (index);
    return current != end ? *current : nullptr;
}

OrderedIndex::ReadCursor &OrderedIndex::ReadCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);

    ++current;
    return *this;
}

OrderedIndex::ReadCursor::ReadCursor (
    OrderedIndex *_index, std::vector <const void *>::const_iterator _begin,
    std::vector <const void *>::const_iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::EditCursor::EditCursor (OrderedIndex::EditCursor &&_other) noexcept
    : index (_other.index),
      current (std::move (_other.current)),
      end (std::move (_other.end))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::EditCursor::~EditCursor () noexcept
{
    if (index)
    {
        if (current != end)
        {
            index->storage->EndRecordEdition (*current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *OrderedIndex::EditCursor::operator * () noexcept
{
    assert (index);
    return current != end ? const_cast <void *> (*current) : nullptr;
}

OrderedIndex::EditCursor &OrderedIndex::EditCursor::operator ~ ()
{
    assert (index);
    assert (current != end);

    index->DeleteRecordMyself (current);
    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::EditCursor &OrderedIndex::EditCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != end);
    index->storage->EndRecordEdition (*current);

    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::EditCursor::EditCursor (
    OrderedIndex *_index, std::vector <const void *>::iterator _begin,
    std::vector <const void *>::iterator _end) noexcept
    : index (_index),
      current (std::move (_begin)),
      end (std::move (_end))
{
    assert (index);
    assert (current <= end);

    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void OrderedIndex::EditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != end)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (const OrderedIndex::ReversedReadCursor &_other) noexcept
    : index (_other.index),
      predecessor (_other.predecessor),
      current (_other.current)
{
    assert (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (OrderedIndex::ReversedReadCursor &&_other) noexcept
    : index (_other.index),
      predecessor (std::move (_other.predecessor)),
      current (std::move (_other.current))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReversedReadCursor::~ReversedReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

OrderedIndex::ReversedEditCursor::ReversedEditCursor (OrderedIndex::ReversedEditCursor &&_other) noexcept
    : index (_other.index),
      predecessor (std::move (_other.predecessor)),
      current (std::move (_other.current))
{
    assert (index);
    _other.index = nullptr;
}

OrderedIndex::ReversedEditCursor::~ReversedEditCursor () noexcept
{
    if (index)
    {
        if (current != predecessor)
        {
            index->storage->EndRecordEdition (*current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *OrderedIndex::ReversedEditCursor::operator * () noexcept
{
    return nullptr;
}

OrderedIndex::ReversedEditCursor &OrderedIndex::ReversedEditCursor::operator ~ ()
{
    assert (index);
    assert (current != predecessor);

    index->DeleteRecordMyself (current);
    --current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::ReversedEditCursor &OrderedIndex::ReversedEditCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != predecessor);
    index->storage->EndRecordEdition (*current);

    ++current;
    BeginRecordEdition ();
    return *this;
}

OrderedIndex::ReversedEditCursor::ReversedEditCursor (
    OrderedIndex *_index, std::vector <const void *>::iterator _predecessor,
    std::vector <const void *>::iterator _last) noexcept
    : index (_index),
      predecessor (std::move (_predecessor)),
      current (std::move (_last))
{
    assert (index);
    assert (predecessor <= current);

    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void OrderedIndex::ReversedEditCursor::BeginRecordEdition () const noexcept
{
    assert (index);
    if (current != predecessor)
    {
        index->storage->BeginRecordEdition (*current);
    }
}

const void *OrderedIndex::ReversedReadCursor::operator * () const noexcept
{
    assert (index);
    return current != predecessor ? *current : nullptr;
}

OrderedIndex::ReversedReadCursor &OrderedIndex::ReversedReadCursor::operator ++ () noexcept
{
    assert (index);
    assert (current != predecessor);

    --current;
    return *this;
}

OrderedIndex::ReversedReadCursor::ReversedReadCursor (
    OrderedIndex *_index, std::vector <const void *>::const_iterator _predecessor,
    std::vector <const void *>::const_iterator _last) noexcept
    : index (_index),
      predecessor (std::move (_predecessor)),
      current (std::move (_last))
{
    assert (index);
    assert (predecessor >= current);

    ++index->activeCursors;
    index->storage->RegisterReader ();
}

OrderedIndex::ReadCursor OrderedIndex::LookupToRead (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReadCursor (this, result.begin, result.end);
}

OrderedIndex::ReversedReadCursor OrderedIndex::LookupToReadReversed (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReversedReadCursor (this, result.begin - 1u, result.end - 1u);
}

OrderedIndex::EditCursor OrderedIndex::LookupToEdit (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::EditCursor (this, result.begin, result.end);
}

OrderedIndex::ReversedEditCursor OrderedIndex::LookupToEditReversed (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result = InternalLookup (_min, _max);
    return OrderedIndex::ReversedEditCursor (this, result.begin - 1u, result.end - 1u);
}

StandardLayout::Field OrderedIndex::GetIndexedField () const noexcept
{
    return indexedField;
}

void OrderedIndex::Drop () noexcept
{
    assert (CanBeDropped ());
    assert (storage);
    storage->DropIndex (*this);
}

bool OrderedIndex::Comparator::operator () (const void *_firstRecord, const void *_secondRecord) const noexcept
{
    assert (_firstRecord);
    assert (_secondRecord);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (field.GetValue (_firstRecord), field.GetValue (_secondRecord), field);
}

bool OrderedIndex::Comparator::operator () (const OrderedIndex::Bound &_bound, const void *_record) const noexcept
{
    assert (_bound.boundValue);
    assert (_record);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (_bound.boundValue, field.GetValue (_record), field);
}

bool OrderedIndex::Comparator::operator () (const void *_record, const OrderedIndex::Bound &_bound) const noexcept
{
    assert (_record);
    assert (_bound.boundValue);
    const StandardLayout::Field &field = owner->indexedField;
    return IsFieldValueLesser (field.GetValue (_record), _bound.boundValue, field);
}

OrderedIndex::OrderedIndex (Storage *_owner, StandardLayout::FieldId _indexedField)
    : IndexBase (_owner),
      indexedField (_owner->GetRecordMapping ().GetField (_indexedField))
{
    assert (indexedField.IsHandleValid ());
}

OrderedIndex::InternalLookupResult OrderedIndex::InternalLookup (
    const OrderedIndex::Bound &_min, const OrderedIndex::Bound &_max) noexcept
{
    InternalLookupResult result {records.begin (), records.end ()};
    if (_min.boundValue)
    {
        result.begin = std::lower_bound (records.begin (), records.end (),
                                         _min, Comparator {this});
    }

    if (_max.boundValue)
    {
        result.end = std::upper_bound (result.begin, records.end (),
                                       _max, Comparator {this});
    }

    return result;
}

void OrderedIndex::InsertRecord (const void *_record) noexcept
{
    // TODO: Assert that record is not inserted already? Is there any way to check this fast?
    assert (_record);
    auto place = std::upper_bound (records.begin (), records.end (), _record);
    records.insert (place, _record);
}

void OrderedIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    // TODO: Implement.
    // Suppress unused.
    _record = nullptr;
    _recordBackup = nullptr;
}

void OrderedIndex::DeleteRecordMyself (std::vector <const void *>::iterator _position) noexcept
{
    // TODO: Implement.
    // Suppress unused.
    const void *ptr = *_position;
    ptr = nullptr;
}

void OrderedIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    // TODO: Implement.
    // Suppress unused.
    _record = nullptr;
    _recordBackup = nullptr;
}

void OrderedIndex::OnWriterClosed () noexcept
{
    // TODO: Implement.
}
} // namespace Emergence::Pegasus