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
    return static_cast <const PlainMapping *> (handle)->objectSize;
}

const FieldMeta *Mapping::GetField (FieldId _field) const noexcept
{
    assert (handle);
    const auto *mapping = static_cast <const PlainMapping *> (handle);

    if (_field < mapping->fieldCount)
    {
        return reinterpret_cast <const FieldMeta *> (mapping + 1u) + _field;
    }
    else
    {
        return nullptr;
    }
}

Mapping::Mapping (void *_handle) noexcept
    : handle (_handle)
{
    assert (handle);
    auto *mapping = static_cast <PlainMapping *> (handle);
    ++mapping->references;
}

Mapping::~Mapping () noexcept
{
    if (handle)
    {
        auto *mapping = static_cast <PlainMapping *> (handle);
        if (mapping->objectSize == 1u)
        {
            free (mapping);
        }
        else
        {
            --mapping->references;
        }
    }
}
} // namespace Emergence::StandardLayout