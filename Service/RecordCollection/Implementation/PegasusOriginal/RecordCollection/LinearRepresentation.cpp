#include <cassert>

#include <Handling/Handle.hpp>

#include <Pegasus/OrderedIndex.hpp>

#include <RecordCollection/LinearRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
LinearRepresentation::AscendingReadCursor::AscendingReadCursor (const LinearRepresentation::AscendingReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (_other.data));
}

LinearRepresentation::AscendingReadCursor::AscendingReadCursor (LinearRepresentation::AscendingReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (_other.data)));
}

LinearRepresentation::AscendingReadCursor::~AscendingReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data).~AscendingReadCursor ();
}

const void *LinearRepresentation::AscendingReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data);
}

LinearRepresentation::AscendingReadCursor &LinearRepresentation::AscendingReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (data);
    return *this;
}

LinearRepresentation::AscendingReadCursor::AscendingReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingReadCursor> (*_data)));
}

LinearRepresentation::AscendingEditCursor::AscendingEditCursor (LinearRepresentation::AscendingEditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (_other.data)));
}

LinearRepresentation::AscendingEditCursor::~AscendingEditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data).~AscendingEditCursor ();
}

void *LinearRepresentation::AscendingEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
}

LinearRepresentation::AscendingEditCursor &LinearRepresentation::AscendingEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
    return *this;
}

LinearRepresentation::AscendingEditCursor &LinearRepresentation::AscendingEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (data);
    return *this;
}

LinearRepresentation::AscendingEditCursor::AscendingEditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::AscendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::AscendingEditCursor> (*_data)));
}

LinearRepresentation::DescendingReadCursor::DescendingReadCursor (
    const LinearRepresentation::DescendingReadCursor &_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (_other.data));
}

LinearRepresentation::DescendingReadCursor::DescendingReadCursor (
    LinearRepresentation::DescendingReadCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (_other.data)));
}

LinearRepresentation::DescendingReadCursor::~DescendingReadCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data).~DescendingReadCursor ();
}

const void *LinearRepresentation::DescendingReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data);
}

LinearRepresentation::DescendingReadCursor &LinearRepresentation::DescendingReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (data);
    return *this;
}

LinearRepresentation::DescendingReadCursor::DescendingReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingReadCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingReadCursor> (*_data)));
}

LinearRepresentation::DescendingEditCursor::DescendingEditCursor (
    LinearRepresentation::DescendingEditCursor &&_other) noexcept
{
    new (&data) Pegasus::OrderedIndex::DescendingEditCursor (
        std::move (block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (_other.data)));
}

LinearRepresentation::DescendingEditCursor::~DescendingEditCursor () noexcept
{
    block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data).~DescendingEditCursor ();
}

void *LinearRepresentation::DescendingEditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
}

LinearRepresentation::DescendingEditCursor &LinearRepresentation::DescendingEditCursor::operator ~ () noexcept
{
    ~block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
    return *this;
}

LinearRepresentation::DescendingEditCursor &LinearRepresentation::DescendingEditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::OrderedIndex::DescendingEditCursor> (data);
    return *this;
}

LinearRepresentation::DescendingEditCursor::DescendingEditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
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

LinearRepresentation::AscendingReadCursor LinearRepresentation::ReadAscendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingReadCursor cursor = index->LookupToReadAscending ({_min}, {_max});
    return AscendingReadCursor (reinterpret_cast <decltype (AscendingReadCursor::data) *> (&cursor));
}

LinearRepresentation::AscendingEditCursor LinearRepresentation::EditAscendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::AscendingEditCursor cursor = index->LookupToEditAscending ({_min}, {_max});
    return AscendingEditCursor (reinterpret_cast <decltype (AscendingEditCursor::data) *> (&cursor));
}

LinearRepresentation::DescendingReadCursor LinearRepresentation::ReadDescendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingReadCursor cursor = index->LookupToReadDescending ({_min}, {_max});

    return DescendingReadCursor (
        reinterpret_cast <decltype (DescendingReadCursor::data) *> (&cursor));
}

LinearRepresentation::DescendingEditCursor LinearRepresentation::EditDescendingInterval (
    LinearRepresentation::KeyFieldValue _min, LinearRepresentation::KeyFieldValue _max) noexcept
{
    assert (handle);
    Pegasus::OrderedIndex *index = reinterpret_cast <Handling::Handle <Pegasus::OrderedIndex> *> (&handle)->Get ();
    Pegasus::OrderedIndex::DescendingEditCursor cursor = index->LookupToEditDescending ({_min}, {_max});

    return DescendingEditCursor (
        reinterpret_cast <decltype (DescendingEditCursor::data) *> (&cursor));
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