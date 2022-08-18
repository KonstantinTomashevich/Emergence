#include <cassert>

#include <API/Common/Implementation/Iterator.hpp>

#include <StandardLayout/Original/PlainPatch.hpp>
#include <StandardLayout/Patch.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
using Iterator = Patch::Iterator;

using IteratorImplementation = const ValueSetter *;

EMERGENCE_BIND_BIDIRECTIONAL_ITERATOR_OPERATIONS_IMPLEMENTATION (Iterator, IteratorImplementation)

Patch::ChangeInfo Patch::Iterator::operator* () const noexcept
{
    const auto *valueSetter = block_cast<const ValueSetter *> (data);
    return {valueSetter->field, &valueSetter->value};
}

Patch::Patch (const Patch &_other) noexcept
    : Patch (_other.data)
{
}

Patch::Patch (Patch &&_other) noexcept
    : Patch (_other.data)
{
}

Patch::~Patch () noexcept
{
    block_cast<Handling::Handle<PlainPatch>> (data).~Handle ();
}

Mapping Patch::GetTypeMapping () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    assert (handle);
    return handle->GetTypeMapping ();
}

void Patch::Apply (void *_object) const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    assert (handle);
    handle->Apply (_object);
}

std::size_t Patch::GetChangeCount () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    assert (handle);
    return handle->GetValueCount ();
}

Iterator Patch::Begin () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    assert (handle);
    const ValueSetter *iterator = handle->Begin ();
    return Iterator (array_cast (iterator));
}

Iterator Patch::End () const noexcept
{
    const auto &handle = block_cast<Handling::Handle<PlainPatch>> (data);
    assert (handle);
    const ValueSetter *iterator = handle->End ();
    return Iterator (array_cast (iterator));
}

Patch Patch::operator+ (const Patch &_other) const noexcept
{
    assert (GetTypeMapping () == _other.GetTypeMapping ());
    PlainPatchBuilder builder;
    builder.Begin (GetTypeMapping ());

    for (const Patch::ChangeInfo &info : *this)
    {
        bool overrideFound = false;

        // Not the most performance-friendly logic, but patches are usually small, so this is good enough.
        for (const Patch::ChangeInfo &overriderInfo : _other)
        {
            if (overriderInfo.field == info.field)
            {
                overrideFound = true;
                break;
            }
        }

        if (!overrideFound)
        {
            builder.Set (info.field, *static_cast<const std::array<uint8_t, VALUE_MAX_SIZE> *> (info.newValue));
        }
    }

    for (const Patch::ChangeInfo &info : _other)
    {
        builder.Set (info.field, *static_cast<const std::array<uint8_t, VALUE_MAX_SIZE> *> (info.newValue));
    }

    Handling::Handle<PlainPatch> patch = builder.End ();
    return Patch (array_cast (patch));
}

Patch::Patch (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (block_cast<Handling::Handle<PlainPatch>> (_data));
}

Patch::Patch (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (std::move (block_cast<Handling::Handle<PlainPatch>> (_data)));
}

Patch::Iterator begin (const Patch &_patch) noexcept
{
    return _patch.Begin ();
}

Patch::Iterator end (const Patch &_patch) noexcept
{
    return _patch.End ();
}
} // namespace Emergence::StandardLayout
