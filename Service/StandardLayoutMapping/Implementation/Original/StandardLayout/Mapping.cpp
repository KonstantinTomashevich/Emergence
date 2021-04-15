#include <cassert>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
// TODO: Is such binding possible without so many casts?
Field Mapping::FieldIterator::operator * () const noexcept
{
    return Field (const_cast <FieldData *> (&**reinterpret_cast <const PlainMapping::ConstIterator *> (&data)));
}

Mapping::FieldIterator &Mapping::FieldIterator::operator ++ () noexcept
{
    ++*reinterpret_cast <PlainMapping::ConstIterator *> (&data);
    return *this;
}

Mapping::FieldIterator Mapping::FieldIterator::operator ++ (int) noexcept
{
    PlainMapping::ConstIterator result = ++*reinterpret_cast <PlainMapping::ConstIterator *> (&data);
    return Mapping::FieldIterator (reinterpret_cast <decltype (data) *> (&result));
}

Mapping::FieldIterator &Mapping::FieldIterator::operator -- () noexcept
{
    --*reinterpret_cast <PlainMapping::ConstIterator *> (&data);
    return *this;
}

Mapping::FieldIterator Mapping::FieldIterator::operator -- (int) noexcept
{
    PlainMapping::ConstIterator result = --*reinterpret_cast <PlainMapping::ConstIterator *> (&data);
    return Mapping::FieldIterator (reinterpret_cast <decltype (data) *> (&result));
}

bool Mapping::FieldIterator::operator == (const Mapping::FieldIterator &_other) const noexcept
{
    return *reinterpret_cast <const PlainMapping::ConstIterator *> (&data) ==
           *reinterpret_cast <const PlainMapping::ConstIterator *> (&_other.data);
}

bool Mapping::FieldIterator::operator != (const Mapping::FieldIterator &_other) const noexcept
{
    return !(*this == _other);
}

Mapping::FieldIterator::FieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    static_assert (sizeof (Mapping::FieldIterator::data) >= sizeof (PlainMapping::ConstIterator));
    new (&data) PlainMapping::ConstIterator (*reinterpret_cast <const PlainMapping::ConstIterator *> (_data));
}

Mapping::FieldIterator::~FieldIterator () noexcept
{
    reinterpret_cast <const PlainMapping::ConstIterator *> (&data)->~ConstIterator ();
}

Mapping::Mapping (const Mapping &_other) noexcept
    : Mapping (_other.handle)
{
}

Mapping::Mapping (Mapping &&_other) noexcept
    : handle (_other.handle)
{
    assert (handle);
    _other.handle = nullptr;
}

std::size_t Mapping::GetObjectSize () const noexcept
{
    assert (handle);
    return static_cast <const PlainMapping *> (handle)->GetObjectSize ();
}

Field Mapping::GetField (FieldId _field) const noexcept
{
    assert (handle);
    return Field (static_cast <PlainMapping *> (handle)->GetField (_field));
}

Mapping::FieldIterator Mapping::Begin () const noexcept
{
    assert (handle);
    PlainMapping::ConstIterator iterator = static_cast <PlainMapping *> (handle)->Begin ();
    return FieldIterator (reinterpret_cast <decltype (FieldIterator::data) *> (&iterator));
}

Mapping::FieldIterator Mapping::End () const noexcept
{
    assert (handle);
    PlainMapping::ConstIterator iterator = static_cast <PlainMapping *> (handle)->End ();
    return FieldIterator (reinterpret_cast <decltype (FieldIterator::data) *> (&iterator));
}

FieldId Mapping::GetFieldId (const Mapping::FieldIterator &_iterator) const noexcept
{
    assert (handle);
    return static_cast <PlainMapping *> (handle)->GetFieldId (
        *reinterpret_cast <const PlainMapping::ConstIterator *> (&_iterator.data));
}

Mapping::Mapping (void *_handle) noexcept
    : handle (_handle)
{
    assert (handle);
    static_cast <PlainMapping *> (handle)->RegisterReference ();
}

Mapping::~Mapping () noexcept
{
    if (handle)
    {
        static_cast <PlainMapping *> (handle)->UnregisterReference ();
    }
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