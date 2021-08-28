#include <cassert>

#include <API/Common/Implementation/Iterator.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
using FieldIterator = Mapping::FieldIterator;

using FieldIteratorImplementation = PlainMapping::ConstIterator;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (FieldIterator, FieldIteratorImplementation)

Field Mapping::FieldIterator::operator* () const noexcept
{
    return Field (const_cast<FieldData *> (&*block_cast<const PlainMapping::ConstIterator> (data)));
}

Mapping::Mapping (const Mapping &_other) noexcept : Mapping (&_other.data)
{
}

Mapping::Mapping (Mapping &&_other) noexcept : Mapping (&_other.data)
{
}

std::size_t Mapping::GetObjectSize () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    assert (handle);
    return handle->GetObjectSize ();
}

Field Mapping::GetField (FieldId _field) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    assert (handle);
    return Field (handle->GetField (_field));
}

bool Mapping::operator== (const Mapping &_other) const noexcept
{
    return block_cast<Handling::Handle<PlainMapping>> (data) ==
           block_cast<Handling::Handle<PlainMapping>> (_other.data);
}

bool Mapping::operator!= (const Mapping &_other) const noexcept
{
    return !(*this == _other);
}

Mapping::FieldIterator Mapping::Begin () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    assert (handle);
    PlainMapping::ConstIterator iterator = handle->Begin ();
    return FieldIterator (reinterpret_cast<decltype (FieldIterator::data) *> (&iterator));
}

Mapping::FieldIterator Mapping::End () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    assert (handle);
    PlainMapping::ConstIterator iterator = handle->End ();
    return FieldIterator (reinterpret_cast<decltype (FieldIterator::data) *> (&iterator));
}

FieldId Mapping::GetFieldId (const Mapping::FieldIterator &_iterator) const noexcept
{
    return GetFieldId (*_iterator);
}

FieldId Mapping::GetFieldId (const Field &_field) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainMapping>> (data);
    assert (handle);
    return handle->GetFieldId (*reinterpret_cast<const FieldData *> (_field.handle));
}

Mapping::Mapping (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    assert (_data);
    new (&data) Handling::Handle<PlainMapping> (block_cast<Handling::Handle<PlainMapping>> (*_data));
}

Mapping::Mapping (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    assert (_data);
    new (&data) Handling::Handle<PlainMapping> (std::move (block_cast<Handling::Handle<PlainMapping>> (*_data)));
}

Mapping::~Mapping () noexcept
{
    block_cast<Handling::Handle<PlainMapping>> (data).~Handle ();
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