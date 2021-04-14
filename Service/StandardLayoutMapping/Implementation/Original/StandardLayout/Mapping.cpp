#include <cassert>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Original/PlainMapping.hpp>

namespace Emergence::StandardLayout
{
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
} // namespace Emergence::StandardLayout