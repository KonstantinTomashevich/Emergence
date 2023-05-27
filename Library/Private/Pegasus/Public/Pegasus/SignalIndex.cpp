#include <algorithm>

#include <Pegasus/SignalIndex.hpp>
#include <Pegasus/Storage.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Pegasus
{
SignalIndex::ReadCursor::ReadCursor (const SignalIndex::ReadCursor &_other) noexcept
    : index (_other.index),
      current (_other.current)
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

SignalIndex::ReadCursor::ReadCursor (SignalIndex::ReadCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current)
{
    EMERGENCE_ASSERT (index);
    _other.index = nullptr;
}

SignalIndex::ReadCursor::~ReadCursor () noexcept
{
    if (index)
    {
        --index->activeCursors;
        index->storage->UnregisterReader ();
    }
}

const void *SignalIndex::ReadCursor::operator* () const noexcept
{
    EMERGENCE_ASSERT (index);
    return current != index->signaledRecords.end () ? *current : nullptr;
}

SignalIndex::ReadCursor &SignalIndex::ReadCursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != index->signaledRecords.end ());
    ++current;
    return *this;
}

SignalIndex::ReadCursor::ReadCursor (SignalIndex *_index) noexcept
    : index (_index),
      current (index->signaledRecords.begin ())
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterReader ();
}

SignalIndex::EditCursor::EditCursor (SignalIndex::EditCursor &&_other) noexcept
    : index (_other.index),
      current (_other.current)
{
    EMERGENCE_ASSERT (index);
    _other.index = nullptr;
}

SignalIndex::EditCursor::~EditCursor () noexcept
{
    if (index)
    {
        if (current != index->signaledRecords.end () && index->storage->EndRecordEdition (*current, index))
        {
            index->OnRecordChangedByMe (current);
        }

        --index->activeCursors;
        index->storage->UnregisterWriter ();
    }
}

void *SignalIndex::EditCursor::operator* () noexcept
{
    EMERGENCE_ASSERT (index);
    return current != index->signaledRecords.end () ? const_cast<void *> (*current) : nullptr;
}

SignalIndex::EditCursor &SignalIndex::EditCursor::operator~() noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != index->signaledRecords.end ());
    index->DeleteRecordMyself (current);
    BeginRecordEdition ();
    return *this;
}

SignalIndex::EditCursor &SignalIndex::EditCursor::operator++ () noexcept
{
    EMERGENCE_ASSERT (index);
    EMERGENCE_ASSERT (current != index->signaledRecords.end ());

    if (index->storage->EndRecordEdition (*current, index))
    {
        index->OnRecordChangedByMe (current);
    }
    else
    {
        ++current;
    }

    BeginRecordEdition ();
    return *this;
}

SignalIndex::EditCursor::EditCursor (SignalIndex *_index) noexcept
    : index (_index),
      current (index->signaledRecords.begin ())
{
    EMERGENCE_ASSERT (index);
    ++index->activeCursors;
    index->storage->RegisterWriter ();
    BeginRecordEdition ();
}

void SignalIndex::EditCursor::BeginRecordEdition () const noexcept
{
    EMERGENCE_ASSERT (index);
    if (current != index->signaledRecords.end ())
    {
        index->storage->BeginRecordEdition (*current);
    }
}

SignalIndex::ReadCursor SignalIndex::LookupSignaledToRead ()
{
    return {this};
}

SignalIndex::EditCursor SignalIndex::LookupSignaledToEdit ()
{
    return {this};
}

const StandardLayout::Field &SignalIndex::GetIndexedField () const
{
    return indexedField;
}

bool SignalIndex::IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const
{
    const std::size_t offsetDelta = indexedField.GetOffset () - offset;
    std::array<std::uint8_t, sizeof (std::uint64_t)> shiftedValue;

    for (std::size_t index = 0u; index < indexedField.GetSize (); ++index)
    {
        shiftedValue[index + offsetDelta] = _value[index];
    }

    auto convertedValue = block_cast<std::uint64_t> (shiftedValue);
    return (convertedValue & mask) == signaledValue;
}

std::array<std::uint8_t, sizeof (std::uint64_t)> SignalIndex::GetSignaledValue () const
{
    const std::size_t offsetDelta = indexedField.GetOffset () - offset;
    std::array<std::uint8_t, sizeof (std::uint64_t)> result;
    std::fill (result.begin (), result.end (), std::uint8_t (0u));

    for (std::size_t index = 0u; index < indexedField.GetSize (); ++index)
    {
        result[index] = array_cast (signaledValue)[index + offsetDelta];
    }

    return result;
}

void SignalIndex::Drop () noexcept
{
    EMERGENCE_ASSERT (CanBeDropped ());
    EMERGENCE_ASSERT (storage);
    storage->DropIndex (*this);
}

using namespace Memory::Literals;

static std::size_t CalculateOffset (const StandardLayout::Field &_field)
{
    EMERGENCE_ASSERT (_field.GetSize () <= sizeof (std::uint64_t));
    std::size_t offset = _field.GetOffset ();

    if (const std::size_t leftover = offset % sizeof (std::uint64_t))
    {
        offset -= sizeof (std::uint64_t) - leftover;
    }

    EMERGENCE_ASSERT (_field.GetOffset () + _field.GetSize () <= offset + sizeof (std::uint64_t));
    return offset;
}

static std::uint64_t CalculateMask (const StandardLayout::Field &_field, std::size_t _offset)
{
    std::array<std::uint8_t, sizeof (std::uint64_t)> maskData;
    std::fill (maskData.begin (), maskData.end (), std::uint8_t (0u));
    const std::size_t offsetDelta = _field.GetOffset () - _offset;

    if (_field.GetArchetype () == StandardLayout::FieldArchetype::BIT)
    {
        maskData[offsetDelta] = 1u << _field.GetBitOffset ();
    }
    else
    {
        EMERGENCE_ASSERT (_field.GetSize () + offsetDelta <= maskData.size ());
        for (std::size_t index = 0u; index < _field.GetSize (); ++index)
        {
            maskData[index + offsetDelta] = 255u;
        }
    }

    return block_cast<std::uint64_t> (maskData);
}

static std::uint64_t CalculateSignaledValue (const StandardLayout::Field &_field,
                                             std::size_t _offset,
                                             std::uint64_t _mask,
                                             const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue)
{
    const std::size_t offsetDelta = _field.GetOffset () - _offset;
    std::array<std::uint8_t, sizeof (std::uint64_t)> shiftedValue;

    for (std::size_t index = 0u; index < _signaledValue.size () - offsetDelta; ++index)
    {
        shiftedValue[index + offsetDelta] = _signaledValue[index];
    }

    return block_cast<std::uint64_t> (shiftedValue) & _mask;
}

SignalIndex::SignalIndex (Storage *_owner,
                          StandardLayout::FieldId _indexedField,
                          const std::array<std::uint8_t, sizeof (std::uint64_t)> &_signaledValue) noexcept
    : IndexBase (_owner),
      indexedField (_owner->GetRecordMapping ().GetField (_indexedField)),
      offset (CalculateOffset (indexedField)),
      mask (CalculateMask (indexedField, offset)),
      signaledValue (CalculateSignaledValue (indexedField, offset, mask, _signaledValue)),
      signaledRecords (Memory::Profiler::AllocationGroup {"Records"_us})
{
}

bool SignalIndex::IsSignaled (const void *_record) const noexcept
{
    const auto *targetAddress = static_cast<const std::uint8_t *> (_record) + offset;
    const std::uint64_t &recordValue = *reinterpret_cast<const std::uint64_t *> (targetAddress);
    return (recordValue & mask) == signaledValue;
}

void SignalIndex::InsertRecord (const void *_record) noexcept
{
    if (IsSignaled (_record))
    {
        signaledRecords.emplace_back (_record);
    }
}

void SignalIndex::OnRecordDeleted (const void *_record, const void *_recordBackup) noexcept
{
    if (IsSignaled (_recordBackup))
    {
        auto iterator = std::find (signaledRecords.begin (), signaledRecords.end (), _record);
        EMERGENCE_ASSERT (iterator != signaledRecords.end ());
        Container::EraseExchangingWithLast (signaledRecords, iterator);
    }
}

void SignalIndex::DeleteRecordMyself (const Container::Vector<const void *>::iterator &_position) noexcept
{
    void *record = const_cast<void *> (*_position);
    Container::EraseExchangingWithLast (signaledRecords, _position);
    storage->DeleteRecord (record, this);
}

void SignalIndex::OnRecordChanged (const void *_record, const void *_recordBackup) noexcept
{
    const bool signaledNow = IsSignaled (_record);
    const bool wasSignaled = IsSignaled (_recordBackup);

    if (signaledNow && !wasSignaled)
    {
        signaledRecords.emplace_back (_record);
    }
    else if (!signaledNow && wasSignaled)
    {
        auto iterator = std::find (signaledRecords.begin (), signaledRecords.end (), _record);
        EMERGENCE_ASSERT (iterator != signaledRecords.end ());
        Container::EraseExchangingWithLast (signaledRecords, iterator);
    }
    else
    {
        EMERGENCE_ASSERT (!signaledNow && !wasSignaled);
    }
}

void SignalIndex::OnRecordChangedByMe (Container::Vector<const void *>::iterator _position) noexcept
{
    // If record was returned by signaled records iterator and indexed field was changed,
    // then the only possible transition is from signaled to unsignaled state.
    EMERGENCE_ASSERT (!IsSignaled (*_position));
    Container::EraseExchangingWithLast (signaledRecords, _position);
}

void SignalIndex::OnWriterClosed () noexcept
{
    // All changes and deletions should be processed on the spot.
}
} // namespace Emergence::Pegasus
