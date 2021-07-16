#include <cassert>

#include <Handling/Handle.hpp>

#include <Pegasus/OrderedIndex.hpp>

#include <RecordCollection/LinearRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
LinearRepresentation::ReadCursor::ReadCursor (const LinearRepresentation::ReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReadCursor (block_cast <Pegasus::OrderedIndex::ReadCursor> (_other.data));
}

LinearRepresentation::ReadCursor::ReadCursor (LinearRepresentation::ReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReadCursor> (_other.data)));
}

LinearRepresentation::ReadCursor::~ReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::ReadCursor> (data).~ReadCursor ();
}

const void *LinearRepresentation::ReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::ReadCursor> (data);
}

LinearRepresentation::ReadCursor &LinearRepresentation::ReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::ReadCursor> (data);
    return *this;
}

LinearRepresentation::ReadCursor::ReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReadCursor> (*_data)));
}

LinearRepresentation::EditCursor::EditCursor (LinearRepresentation::EditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::EditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::EditCursor> (_other.data)));
}

LinearRepresentation::EditCursor::~EditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::EditCursor> (data).~EditCursor ();
}

void *LinearRepresentation::EditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::EditCursor> (data);
}

LinearRepresentation::EditCursor &LinearRepresentation::EditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::EditCursor> (data);
    return *this;
}

LinearRepresentation::EditCursor &LinearRepresentation::EditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::EditCursor> (data);
    return *this;
}

LinearRepresentation::EditCursor::EditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::EditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::EditCursor> (*_data)));
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (
    const LinearRepresentation::ReversedReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReversedReadCursor (
        block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (_other.data));
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (
    LinearRepresentation::ReversedReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReversedReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (_other.data)));
}

LinearRepresentation::ReversedReadCursor::~ReversedReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (data).~ReversedReadCursor ();
}

const void *LinearRepresentation::ReversedReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (data);
}

LinearRepresentation::ReversedReadCursor &LinearRepresentation::ReversedReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (data);
    return *this;
}

LinearRepresentation::ReversedReadCursor::ReversedReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReversedReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReversedReadCursor> (*_data)));
}

LinearRepresentation::ReversedEditCursor::ReversedEditCursor (
    LinearRepresentation::ReversedEditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReversedEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (_other.data)));
}

LinearRepresentation::ReversedEditCursor::~ReversedEditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (data).~ReversedEditCursor ();
}

void *LinearRepresentation::ReversedEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (data);
}

LinearRepresentation::ReversedEditCursor &LinearRepresentation::ReversedEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (data);
    return *this;
}

LinearRepresentation::ReversedEditCursor &LinearRepresentation::ReversedEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (data);
    return *this;
}

LinearRepresentation::ReversedEditCursor::ReversedEditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::ReversedEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::ReversedEditCursor> (*_data)));
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
    Pegasus::OrderedIndex::ReadCursor cursor = index->LookupToRead ({_min}, {_max});
    return ReadCursor (reinterpret_cast <decltype (ReadCursor::data) *> (&cursor));
}

LinearRepresentation::EditCursor LinearRepresentation::EditInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::EditCursor cursor = index->LookupToEdit ({_min}, {_max});
    return EditCursor (reinterpret_cast <decltype (EditCursor::data) *> (&cursor));
}

LinearRepresentation::ReversedReadCursor LinearRepresentation::ReadReversedInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::ReversedReadCursor cursor = index->LookupToReadReversed ({_min}, {_max});

    return ReversedReadCursor (
        reinterpret_cast <decltype (ReversedReadCursor::data) *> (&cursor));
}

LinearRepresentation::ReversedEditCursor LinearRepresentation::EditReversedInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::ReversedEditCursor cursor = index->LookupToEditReversed ({_min}, {_max});

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
    return reinterpret_cast <const Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ()->CanBeDropped ();
}

void LinearRepresentation::Drop () noexcept
{
    assert (handle);
    return reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ()->Drop ();
}

LinearRepresentation::LinearRepresentation (void *_handle) noexcept
{
    static_assert (sizeof (handle) == sizeof (Handling::Handle <Pegasus::OrderedIndex>));
    new (&handle) Handling::Handle <Pegasus::OrderedIndex> (static_cast <Pegasus::OrderedIndex *> (_handle));
}
} // namespace Emergence::RecordCollection