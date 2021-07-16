#include <cassert>

#include <Handling/Handle.hpp>

#include <Pegasus/HashIndex.hpp>

#include <RecordCollection/PointRepresentation.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::RecordCollection
{
PointRepresentation::ReadCursor::ReadCursor (const PointRepresentation::ReadCursor &_other) noexcept
{
    new (&data) Pegasus::HashIndex::ReadCursor (block_cast <Pegasus::HashIndex::ReadCursor> (_other.data));
}

PointRepresentation::ReadCursor::ReadCursor (PointRepresentation::ReadCursor &&_other) noexcept
{
    new (&data) Pegasus::HashIndex::ReadCursor (
        std::move (block_cast <Pegasus::HashIndex::ReadCursor> (_other.data)));
}

PointRepresentation::ReadCursor::~ReadCursor () noexcept
{
    block_cast <Pegasus::HashIndex::ReadCursor> (data).~ReadCursor ();
}

const void *PointRepresentation::ReadCursor::operator * () const noexcept
{
    return *block_cast <Pegasus::HashIndex::ReadCursor> (data);
}

PointRepresentation::ReadCursor &PointRepresentation::ReadCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::HashIndex::ReadCursor> (data);
    return *this;
}

PointRepresentation::ReadCursor::ReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::HashIndex::ReadCursor (std::move (block_cast <Pegasus::HashIndex::ReadCursor> (*_data)));
}

PointRepresentation::EditCursor::EditCursor (PointRepresentation::EditCursor &&_other) noexcept
{
    new (&data) Pegasus::HashIndex::EditCursor (
        std::move (block_cast <Pegasus::HashIndex::EditCursor> (_other.data)));
}

PointRepresentation::EditCursor::~EditCursor () noexcept
{
    block_cast <Pegasus::HashIndex::EditCursor> (data).~EditCursor ();
}

void *PointRepresentation::EditCursor::operator * () noexcept
{
    return *block_cast <Pegasus::HashIndex::EditCursor> (data);
}

PointRepresentation::EditCursor &PointRepresentation::EditCursor::operator ~ ()
{
    ~block_cast <Pegasus::HashIndex::EditCursor> (data);
    return *this;
}

PointRepresentation::EditCursor &PointRepresentation::EditCursor::operator ++ () noexcept
{
    ++block_cast <Pegasus::HashIndex::EditCursor> (data);
    return *this;
}

PointRepresentation::EditCursor::EditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) Pegasus::HashIndex::EditCursor (std::move (block_cast <Pegasus::HashIndex::EditCursor> (*_data)));
}

using KeyFieldIteratorBaseType = InplaceVector <
    StandardLayout::Field, Pegasus::Constants::HashIndex::MAX_INDEXED_FIELDS>::ConstIterator;

PointRepresentation::KeyFieldIterator::KeyFieldIterator (const PointRepresentation::KeyFieldIterator &_other) noexcept
{
    new (&data) KeyFieldIteratorBaseType (block_cast <KeyFieldIteratorBaseType> (_other.data));
}

PointRepresentation::KeyFieldIterator::KeyFieldIterator (PointRepresentation::KeyFieldIterator &&_other) noexcept
{
    new (&data) KeyFieldIteratorBaseType (std::move (block_cast <KeyFieldIteratorBaseType> (_other.data)));
}

PointRepresentation::KeyFieldIterator::~KeyFieldIterator () noexcept
{
    block_cast <KeyFieldIteratorBaseType> (data).~KeyFieldIteratorBaseType ();
}

StandardLayout::Field PointRepresentation::KeyFieldIterator::operator * () const noexcept
{
    return *block_cast <KeyFieldIteratorBaseType> (data);
}

PointRepresentation::KeyFieldIterator &PointRepresentation::KeyFieldIterator::operator ++ () noexcept
{
    ++block_cast <KeyFieldIteratorBaseType> (data);
    return *this;
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldIterator::operator ++ (int) noexcept
{
    auto previous = block_cast <KeyFieldIteratorBaseType> (data)++;
    return KeyFieldIterator (reinterpret_cast <decltype (data) *> (&previous));
}

PointRepresentation::KeyFieldIterator &PointRepresentation::KeyFieldIterator::operator -- () noexcept
{
    --block_cast <KeyFieldIteratorBaseType> (data);
    return *this;
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldIterator::operator -- (int) noexcept
{
    auto previous = block_cast <KeyFieldIteratorBaseType> (data)--;
    return KeyFieldIterator (reinterpret_cast <decltype (data) *> (&previous));
}

bool PointRepresentation::KeyFieldIterator::operator == (
    const PointRepresentation::KeyFieldIterator &_other) const noexcept
{
    return block_cast <KeyFieldIteratorBaseType> (data) == block_cast <KeyFieldIteratorBaseType> (_other.data);
}

bool PointRepresentation::KeyFieldIterator::operator != (
    const PointRepresentation::KeyFieldIterator &_other) const noexcept
{
    return !(*this == _other);
}

PointRepresentation::KeyFieldIterator::KeyFieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) KeyFieldIteratorBaseType (block_cast <KeyFieldIteratorBaseType> (*_data));
}

PointRepresentation::~PointRepresentation () noexcept
{
    if (handle)
    {
        reinterpret_cast <Handling::Handle <Pegasus::HashIndex> *> (&handle)->~Handle ();
    }
}

PointRepresentation::ReadCursor PointRepresentation::ReadPoint (PointRepresentation::Point _point) noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast <Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ();
    Pegasus::HashIndex::ReadCursor cursor = index->LookupToRead ({_point});
    return ReadCursor (reinterpret_cast <decltype (ReadCursor::data) *> (&cursor));
}

PointRepresentation::EditCursor PointRepresentation::EditPoint (PointRepresentation::Point _point) noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast <Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ();
    Pegasus::HashIndex::EditCursor cursor = index->LookupToEdit ({_point});
    return EditCursor (reinterpret_cast <decltype (EditCursor::data) *> (&cursor));
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldBegin () const noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast <const Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ();
    auto iterator = index->GetIndexedFields ().Begin ();
    return KeyFieldIterator (reinterpret_cast <decltype (KeyFieldIterator::data) *> (&iterator));
}

PointRepresentation::KeyFieldIterator PointRepresentation::KeyFieldEnd () const noexcept
{
    assert (handle);
    Pegasus::HashIndex *index = reinterpret_cast <const Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ();
    auto iterator = index->GetIndexedFields ().End ();
    return KeyFieldIterator (reinterpret_cast <decltype (KeyFieldIterator::data) *> (&iterator));
}

bool PointRepresentation::CanBeDropped () const noexcept
{
    assert (handle);
    return reinterpret_cast <const Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ()->CanBeDropped ();
}

void PointRepresentation::Drop () noexcept
{
    assert (handle);
    reinterpret_cast <Handling::Handle <Pegasus::HashIndex> *> (&handle)->Get ()->Drop ();
}

PointRepresentation::PointRepresentation (void *_handle) noexcept
{
    assert (_handle);
    static_assert (sizeof (handle) == sizeof (Handling::Handle <Pegasus::HashIndex>));
    new (&handle) Handling::Handle <Pegasus::HashIndex> (static_cast <Pegasus::HashIndex *> (_handle));
}
} // namespace Emergence::RecordCollection