#include <cassert>

#include <Handling/Handle.hpp>

#include <Pegasus/OrderedIndex.hpp>

#include <RecordCollection/LinearRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
LinearRepresentation::ReadCursor::ReadCursor (const LinearRepresentation::ReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (_other.data));
}

LinearRepresentation::ReadCursor::ReadCursor (LinearRepresentation::ReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (_other.data)));
}

LinearRepresentation::ReadCursor::~ReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data).~AscendingReadCursor ();
}

const void *LinearRepresentation::ReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data);
}

LinearRepresentation::ReadCursor &LinearRepresentation::ReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data);
    return *this;
}

LinearRepresentation::ReadCursor::ReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (*_data)));
}

LinearRepresentation::EditCursor::EditCursor (LinearRepresentation::EditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (_other.data)));
}

LinearRepresentation::EditCursor::~EditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data).~AscendingEditCursor ();
}

void *LinearRepresentation::EditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
}

LinearRepresentation::EditCursor &LinearRepresentation::EditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
    return *this;
}

LinearRepresentation::EditCursor &LinearRepresentation::EditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
    return *this;
}

LinearRepresentation::EditCursor::EditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (*_data)));
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (
    const LinearRepresentation::ReversedReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (_other.data));
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (
    LinearRepresentation::ReversedReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (_other.data)));
}

LinearRepresentation::ReversedReadCursor::~ReversedReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data).~DescendingReadCursor ();
}

const void *LinearRepresentation::ReversedReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data);
}

LinearRepresentation::ReversedReadCursor &LinearRepresentation::ReversedReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data);
    return *this;
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (*_data)));
}

LinearRepresentation::ReversedEditCursor::ReversedEditCursor (
    LinearRepresentation::ReversedEditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (_other.data)));
}

LinearRepresentation::ReversedEditCursor::~ReversedEditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data).~DescendingEditCursor ();
}

void *LinearRepresentation::ReversedEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
}

LinearRepresentation::ReversedEditCursor &LinearRepresentation::ReversedEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
    return *this;
}

LinearRepresentation::ReversedEditCursor &LinearRepresentation::ReversedEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
    return *this;
}

LinearRepresentation::ReversedEditCursor::ReversedEditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (*_data)));
}

LinearRepresentation::LinearRepresentation (const LinearRepresentation &_other) noexcept
{
    new (&handle) Handling::Handle <Pegasus::OrderedIndex> (
        *reinterpret_cast <const Handling::Handle <Pegasus::OrderedIndex> *> (&_other.handle));
}

LinearRepresentation::LinearRepresentation (LinearRepresentation &&_other) noexcept
{
    new (&handle) Handling::Handle <Pegasus::OrderedIndex> (
        std::move (*reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&_other.handle)));
}

LinearRepresentation::~LinearRepresentation () noexcept
{
    reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->~Handle ();
}

LinearRepresentation::ReadCursor LinearRepresentation::ReadInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingReadCursor cursor = index->LookupToReadAscending ({_min}, {_max});
    return ReadCursor (reinterpret_cast <decltype (ReadCursor::data) *> (&cursor));
}

LinearRepresentation::EditCursor LinearRepresentation::EditInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingEditCursor cursor = index->LookupToEditAscending ({_min}, {_max});
    return EditCursor (reinterpret_cast <decltype (EditCursor::data) *> (&cursor));
}

LinearRepresentation::ReversedReadCursor LinearRepresentation::ReadReversedInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingReadCursor cursor = index->LookupToReadDescending ({_min}, {_max});

    return ReversedReadCursor (
        reinterpret_cast <decltype (ReversedReadCursor::data) *> (&cursor));
}

LinearRepresentation::ReversedEditCursor LinearRepresentation::EditReversedInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingEditCursor cursor = index->LookupToEditDescending ({_min}, {_max});

    return ReversedEditCursor (
        reinterpret_cast <decltype (ReversedEditCursor::data) *> (&cursor));
}

StandardLayout::Field LinearRepresentation::GetKeyField () const noexcept
{
    assert (handle);
    return reinterpret_cast <const Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ()->GetIndexedField ();
}

bool LinearRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast <const Handling::Handle <Pegasus::OrderedIndex> *> (&handle);
    Pegasus::OrderedIndex *index = realHandle.Get ();

    // To extract correct result we must temporary unlink index handle.
    const_cast <Handling::Handle <Pegasus::OrderedIndex> &> (realHandle) = nullptr;
    bool canBeDropped = index->CanBeDropped ();
    const_cast <Handling::Handle <Pegasus::OrderedIndex> &> (realHandle) = index;

    return canBeDropped;
}

void LinearRepresentation::Drop () noexcept
{
    assert (handle);
    auto &realHandle = *reinterpret_cast <const Handling::Handle <Pegasus::OrderedIndex> *> (&handle);
    Pegasus::OrderedIndex *index = realHandle.Get ();

    // Free handle first, because indices can not be deleted while any handle points to them.
    const_cast <Handling::Handle <Pegasus::OrderedIndex> &> (realHandle) = nullptr;
    index->Drop ();
}

bool LinearRepresentation::operator == (const LinearRepresentation &_other) const noexcept
{
    return handle == _other.handle;
}

LinearRepresentation &LinearRepresentation::operator = (const LinearRepresentation &_other) noexcept
{
    if (this != &_other)
    {
        this->~LinearRepresentation ();
        new (this) LinearRepresentation (_other);
    }

    return *this;
}

LinearRepresentation &LinearRepresentation::operator = (LinearRepresentation &&_other) noexcept
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
    static_assert (sizeof (handle) == sizeof (Handling::Handle <Pegasus::OrderedIndex>));
    new (&handle) Handling::Handle <Pegasus::OrderedIndex> (static_cast <Pegasus::OrderedIndex *> (_handle));
}
} // namespace Emergence::RecordCollection