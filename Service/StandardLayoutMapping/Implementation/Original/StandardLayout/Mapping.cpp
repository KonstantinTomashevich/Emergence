#include <cassert>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
// TODO: Move trivial iterator binding to macro too?

Mapping::FieldIterator::FieldIterator (const Mapping::FieldIterator &_other) noexcept
{
    new (&data) PlainMapping::ConstIterator (block_cast <PlainMapping::ConstIterator> (_other.data));
}

Mapping::FieldIterator::FieldIterator (Mapping::FieldIterator &&_other) noexcept
{
    new (&data) PlainMapping::ConstIterator (std::move (block_cast <PlainMapping::ConstIterator> (_other.data)));
}

Mapping::FieldIterator::~FieldIterator () noexcept
{
    block_cast <PlainMapping::ConstIterator> (data).~ConstIterator ();
}

Field Mapping::FieldIterator::operator * () const noexcept
{
    return Field (const_cast <FieldData *> (&*block_cast <const PlainMapping::ConstIterator> (data)));
}

Mapping::FieldIterator &Mapping::FieldIterator::operator ++ () noexcept
{
    ++block_cast <PlainMapping::ConstIterator> (data);
    return *this;
}

Mapping::FieldIterator Mapping::FieldIterator::operator ++ (int) noexcept
{
    PlainMapping::ConstIterator result = block_cast <PlainMapping::ConstIterator> (data)++;
    return Mapping::FieldIterator (reinterpret_cast <decltype (data) *> (&result));
}

Mapping::FieldIterator &Mapping::FieldIterator::operator -- () noexcept
{
    --block_cast <PlainMapping::ConstIterator> (data);
    return *this;
}

Mapping::FieldIterator Mapping::FieldIterator::operator -- (int) noexcept
{
    PlainMapping::ConstIterator result = block_cast <PlainMapping::ConstIterator> (data)--;
    return Mapping::FieldIterator (reinterpret_cast <decltype (data) *> (&result));
}

bool Mapping::FieldIterator::operator == (const Mapping::FieldIterator &_other) const noexcept
{
    return block_cast <PlainMapping::ConstIterator> (data) == block_cast <PlainMapping::ConstIterator> (_other.data);
}

bool Mapping::FieldIterator::operator != (const Mapping::FieldIterator &_other) const noexcept
{
    return !(*this == _other);
}

Mapping::FieldIterator &Mapping::FieldIterator::operator = (const Mapping::FieldIterator &_other) noexcept
{
    if (this != &_other)
    {
        this->~FieldIterator ();
        new (this) FieldIterator (_other);
    }

    return *this;
}

Mapping::FieldIterator &Mapping::FieldIterator::operator = (Mapping::FieldIterator &&_other) noexcept
{
    if (this != &_other)
    {
        this->~FieldIterator ();
        new (this) FieldIterator (std::move (_other));
    }

    return *this;
}

Mapping::FieldIterator::FieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) PlainMapping::ConstIterator (block_cast <PlainMapping::ConstIterator> (*_data));
}

Mapping::Mapping (const Mapping &_other) noexcept
    : Mapping (&_other.data)
{
}

Mapping::Mapping (Mapping &&_other) noexcept
    : Mapping (&_other.data)
{
}

std::size_t Mapping::GetObjectSize () const noexcept
{
    const auto &handle = block_cast <Handling::Handle <PlainMapping>> (data);
    assert (handle);
    return handle->GetObjectSize ();
}

Field Mapping::GetField (FieldId _field) const noexcept
{
    const auto &handle = block_cast <Handling::Handle <PlainMapping>> (data);
    assert (handle);
    return Field (handle->GetField (_field));
}

Mapping &Mapping::operator = (const Mapping &_other) noexcept
{
    if (this != &_other)
    {
        this->~Mapping ();
        new (this) Mapping (_other);
    }

    return *this;
}

Mapping &Mapping::operator = (Mapping &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Mapping ();
        new (this) Mapping (std::move (_other));
    }

    return *this;
}

bool Mapping::operator == (const Mapping &_other) const noexcept
{
    return block_cast <Handling::Handle <PlainMapping>> (data) ==
           block_cast <Handling::Handle <PlainMapping>> (_other.data);
}

bool Mapping::operator != (const Mapping &_other) const noexcept
{
    return !(*this == _other);
}

Mapping::FieldIterator Mapping::Begin () const noexcept
{
    const auto &handle = block_cast <Handling::Handle <PlainMapping>> (data);
    assert (handle);
    PlainMapping::ConstIterator iterator = handle->Begin ();
    return FieldIterator (reinterpret_cast <decltype (FieldIterator::data) *> (&iterator));
}

Mapping::FieldIterator Mapping::End () const noexcept
{
    const auto &handle = block_cast <Handling::Handle <PlainMapping>> (data);
    assert (handle);
    PlainMapping::ConstIterator iterator = handle->End ();
    return FieldIterator (reinterpret_cast <decltype (FieldIterator::data) *> (&iterator));
}

FieldId Mapping::GetFieldId (const Mapping::FieldIterator &_iterator) const noexcept
{
    const auto &handle = block_cast <Handling::Handle <PlainMapping>> (data);
    assert (handle);
    return handle->GetFieldId (*reinterpret_cast <const PlainMapping::ConstIterator *> (&_iterator.data));
}

Mapping::Mapping (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    assert (_data);
    new (&data) Handling::Handle <PlainMapping> (block_cast <Handling::Handle <PlainMapping>> (*_data));
}

Mapping::Mapping (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    assert (_data);
    new (&data) Handling::Handle <PlainMapping> (std::move (block_cast <Handling::Handle <PlainMapping>> (*_data)));
}

Mapping::~Mapping () noexcept
{
    block_cast <Handling::Handle <PlainMapping>> (data).~Handle ();
}

Mapping::FieldIterator begin (const Mapping &_mapping) noexcept
{
    return _mapping.Begin ();
}

Mapping::FieldIterator end (const Mapping &_mapping) noexcept
{
    return _mapping.End ();
}
} // namespace Emergence::StandardLayout