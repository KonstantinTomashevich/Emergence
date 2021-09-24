#include <cassert>

#include <API/Common/Implementation/Cursor.hpp>
#include <API/Common/Implementation/Iterator.hpp>

#include <Handling/Handle.hpp>

#include <Pegasus/HashIndex.hpp>

#include <RecordCollection/PointRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
using ReadCursor = PointRepresentation::ReadCursor;

using ReadCursorImplementation = Pegasus::HashIndex::ReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (ReadCursor, ReadCursorImplementation)

using EditCursor = PointRepresentation::EditCursor;

using EditCursorImplementation = Pegasus::HashIndex::EditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (EditCursor, EditCursorImplementation)

using KeyFieldIterator = PointRepresentation::KeyFieldIterator;

using KeyFieldIteratorImplementation =
    InplaceVector<StandardLayout::Field, Pegasus::Constants::HashIndex::MAX_INDEXED_FIELDS>::ConstIterator;

// NOLINTNEXTLINE(modernize-use-auto): It's impossible to use auto there.
EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (KeyFieldIterator, KeyFieldIteratorImplementation)

StandardLayout::Field PointRepresentation::KeyFieldIterator::operator* () const noexcept
{
    return *block_cast<KeyFieldIteratorImplementation> (data);
}

PointRepresentation::PointRepresentation (const PointRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::HashIndex> (
        *reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&_other.handle));
}

PointRepresentation::PointRepresentation (PointRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::HashIndex> (
        std::move (*reinterpret_cast<Handling::Handle<Pegasus::HashIndex> *> (&_other.handle)));
}

PointRepresentation::~PointRepresentation () noexcept
{
    if (handle)
    {
        reinterpret_cast<Handling::Handle<Pegasus::HashIndex> *> (&handle)->~Handle ();
    }
}

PointRepresentation::ReadCursor PointRepresentation::ReadPoint (PointRepresentation::Point _point) noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast<Handling::Handle<Pegasus::HashIndex> *> (&handle)->Get ();
    Pegasus::HashIndex::ReadCursor cursor = index->LookupToRead ({_point});
    return ReadCursor (reinterpret_cast<decltype (ReadCursor::data) *> (&cursor));
}

PointRepresentation::EditCursor PointRepresentation::EditPoint (PointRepresentation::Point _point) noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast<Handling::Handle<Pegasus::HashIndex> *> (&handle)->Get ();
    Pegasus::HashIndex::EditCursor cursor = index->LookupToEdit ({_point});
    return EditCursor (reinterpret_cast<decltype (EditCursor::data) *> (&cursor));
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldBegin () const noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&handle)->Get ();
    auto iterator = index->GetIndexedFields ().Begin ();
    return KeyFieldIterator (reinterpret_cast<decltype (KeyFieldIterator::data) *> (&iterator));
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldEnd () const noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&handle)->Get ();
    auto iterator = index->GetIndexedFields ().End ();
    return KeyFieldIterator (reinterpret_cast<decltype (KeyFieldIterator::data) *> (&iterator));
}

const StandardLayout::Mapping &PointRepresentation::GetTypeMapping () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&handle)->Get ()->GetRecordMapping ();
}

bool PointRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&handle);
    Pegasus::HashIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast<Handling::Handle<Pegasus::HashIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast<Handling::Handle<Pegasus::HashIndex> &> (realHandle) = index;

    return canBeDropped;
}

void PointRepresentation::Drop () noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::HashIndex> *> (&handle);
    Pegasus::HashIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast<Handling::Handle<Pegasus::HashIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool PointRepresentation::operator== (const PointRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

PointRepresentation &PointRepresentation::operator= (const PointRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~PointRepresentation ();
        new (this) PointRepresentation (_other);
    }

    return *this;
}

PointRepresentation &PointRepresentation::operator= (PointRepresentation &&_other) noexcept
{
    if (this != &_other)
    {
        this->~PointRepresentation ();
        new (this) PointRepresentation (std::move (_other));
    }

    return *this;
}

PointRepresentation::PointRepresentation (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (Handling::Handle<Pegasus::HashIndex>));
    new (&handle) Handling::Handle<Pegasus::HashIndex> (static_cast<Pegasus::HashIndex *> (_handle));
}
} // namespace Emergence::RecordCollection
