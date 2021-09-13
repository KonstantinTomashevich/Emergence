#include <cassert>

#include <API/Common/Implementation/Cursor.hpp>

#include <Handling/Handle.hpp>

#include <Pegasus/OrderedIndex.hpp>

#include <RecordCollection/LinearRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
using AscendingReadCursor = LinearRepresentation::AscendingReadCursor;

using AscendingReadCursorImplementation = Pegasus::OrderedIndex::AscendingReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (AscendingReadCursor, AscendingReadCursorImplementation)

using AscendingEditCursor = LinearRepresentation::AscendingEditCursor;

using AscendingEditCursorImplementation = Pegasus::OrderedIndex::AscendingEditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (AscendingEditCursor, AscendingEditCursorImplementation)

using DescendingReadCursor = LinearRepresentation::DescendingReadCursor;

using DescendingReadCursorImplementation = Pegasus::OrderedIndex::DescendingReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (DescendingReadCursor, DescendingReadCursorImplementation)

using DescendingEditCursor = LinearRepresentation::DescendingEditCursor;

using DescendingEditCursorImplementation = Pegasus::OrderedIndex::DescendingEditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (DescendingEditCursor, DescendingEditCursorImplementation)

LinearRepresentation::LinearRepresentation (const LinearRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::OrderedIndex> (
        *reinterpret_cast<const Handling::Handle<Pegasus::OrderedIndex> *> (&_other.handle));
}

LinearRepresentation::LinearRepresentation (LinearRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::OrderedIndex> (
        std::move (*reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&_other.handle)));
}

LinearRepresentation::~LinearRepresentation () noexcept
{
    reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->~Handle ();
}

LinearRepresentation::AscendingReadCursor LinearRepresentation::ReadAscendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingReadCursor cursor = index->LookupToReadAscending ({_min}, {_max});
    return AscendingReadCursor (reinterpret_cast<decltype (AscendingReadCursor::data) *> (&cursor));
}

LinearRepresentation::AscendingEditCursor LinearRepresentation::EditAscendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingEditCursor cursor = index->LookupToEditAscending ({_min}, {_max});
    return AscendingEditCursor (reinterpret_cast<decltype (AscendingEditCursor::data) *> (&cursor));
}

LinearRepresentation::DescendingReadCursor LinearRepresentation::ReadDescendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingReadCursor cursor = index->LookupToReadDescending ({_min}, {_max});

    return DescendingReadCursor (reinterpret_cast<decltype (DescendingReadCursor::data) *> (&cursor));
}

LinearRepresentation::DescendingEditCursor LinearRepresentation::EditDescendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast<Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingEditCursor cursor = index->LookupToEditDescending ({_min}, {_max});

    return DescendingEditCursor (reinterpret_cast<decltype (DescendingEditCursor::data) *> (&cursor));
}

StandardLayout::Field LinearRepresentation::GetKeyField () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ()->GetIndexedField ();
}

const StandardLayout::Mapping &LinearRepresentation::GetTypeMapping () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::OrderedIndex> *> (&handle)->Get ()->GetRecordMapping ();
}

bool LinearRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::OrderedIndex> *> (&handle);
    Pegasus::OrderedIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast<Handling::Handle<Pegasus::OrderedIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast<Handling::Handle<Pegasus::OrderedIndex> &> (realHandle) = index;

    return canBeDropped;
}

void LinearRepresentation::Drop () noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::OrderedIndex> *> (&handle);
    Pegasus::OrderedIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast<Handling::Handle<Pegasus::OrderedIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool LinearRepresentation::operator== (const LinearRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

LinearRepresentation &LinearRepresentation::operator= (const LinearRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~LinearRepresentation ();
        new (this) LinearRepresentation (_other);
    }

    return *this;
}

LinearRepresentation &LinearRepresentation::operator= (LinearRepresentation &&_other) noexcept
{
    if (this != &_other)
    {
        this->~LinearRepresentation ();
        new (this) LinearRepresentation (std::move (_other));
    }

    return *this;
}

LinearRepresentation::LinearRepresentation (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (Handling::Handle<Pegasus::OrderedIndex>));
    new (&handle) Handling::Handle<Pegasus::OrderedIndex> (static_cast<Pegasus::OrderedIndex *> (_handle));
}
} // namespace Emergence::RecordCollection
