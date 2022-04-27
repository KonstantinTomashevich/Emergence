#include <cassert>

#include <API/Common/Implementation/Cursor.hpp>

#include <Handling/Handle.hpp>

#include <Pegasus/SignalIndex.hpp>

#include <RecordCollection/SignalRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
using ReadCursor = SignalRepresentation::ReadCursor;

using ReadCursorImplementation = Pegasus::SignalIndex::ReadCursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (ReadCursor, ReadCursorImplementation)

using EditCursor = SignalRepresentation::EditCursor;

using EditCursorImplementation = Pegasus::SignalIndex::EditCursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (EditCursor, EditCursorImplementation)

SignalRepresentation::SignalRepresentation (const SignalRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::SignalIndex> (
        *reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&_other.handle));
}

SignalRepresentation::SignalRepresentation (SignalRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle<Pegasus::SignalIndex> (
        std::move (*reinterpret_cast<Handling::Handle<Pegasus::SignalIndex> *> (&_other.handle)));
}

SignalRepresentation::~SignalRepresentation () noexcept
{
    reinterpret_cast<Handling::Handle<Pegasus::SignalIndex> *> (&handle)->~Handle ();
}

ReadCursor SignalRepresentation::ReadSignaled () noexcept
{
    assert (handle);
    Pegasus::SignalIndex *index = reinterpret_cast<Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ();
    Pegasus::SignalIndex::ReadCursor cursor = index->LookupSignaledToRead ();
    return ReadCursor (reinterpret_cast<decltype (ReadCursor::data) *> (&cursor));
}

EditCursor SignalRepresentation::EditSignaled () noexcept
{
    assert (handle);
    Pegasus::SignalIndex *index = reinterpret_cast<Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ();
    Pegasus::SignalIndex::EditCursor cursor = index->LookupSignaledToEdit ();
    return EditCursor (reinterpret_cast<decltype (EditCursor::data) *> (&cursor));
}

StandardLayout::Field SignalRepresentation::GetKeyField () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ()->GetIndexedField ();
}

bool SignalRepresentation::IsSignaledValue (const std::array<uint8_t, sizeof (uint64_t)> &_value) const
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ()->IsSignaledValue (_value);
}

std::array<uint8_t, sizeof (uint64_t)> SignalRepresentation::GetSignaledValue () const
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ()->GetSignaledValue ();
}

const StandardLayout::Mapping &SignalRepresentation::GetTypeMapping () const noexcept
{
    assert (handle);
    return reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle)->Get ()->GetRecordMapping ();
}

bool SignalRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle);
    Pegasus::SignalIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast<Handling::Handle<Pegasus::SignalIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast<Handling::Handle<Pegasus::SignalIndex> &> (realHandle) = index;

    return canBeDropped;
}

void SignalRepresentation::Drop () noexcept
{
    assert (handle);
    const auto &realHandle = *reinterpret_cast<const Handling::Handle<Pegasus::SignalIndex> *> (&handle);
    Pegasus::SignalIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast<Handling::Handle<Pegasus::SignalIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool SignalRepresentation::operator== (const SignalRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

SignalRepresentation &SignalRepresentation::operator= (const SignalRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~SignalRepresentation ();
        new (this) SignalRepresentation (_other);
    }

    return *this;
}

SignalRepresentation &SignalRepresentation::operator= (SignalRepresentation &&_other) noexcept
{
    if (this != &_other)
    {
        this->~SignalRepresentation ();
        new (this) SignalRepresentation (std::move (_other));
    }

    return *this;
}

SignalRepresentation::SignalRepresentation (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (Handling::Handle<Pegasus::SignalIndex>));
    new (&handle) Handling::Handle<Pegasus::SignalIndex> (static_cast<Pegasus::SignalIndex *> (_handle));
}
} // namespace Emergence::RecordCollection
