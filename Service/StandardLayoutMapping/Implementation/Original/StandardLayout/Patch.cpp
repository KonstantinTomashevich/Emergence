#include <cassert>

#include <StandardLayout/Original/PlainPatch.hpp>
#include <StandardLayout/Patch.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::StandardLayout
{
Patch::Patch (const Patch &_other) noexcept : Patch (_other.data)
{
}

Patch::Patch (Patch &&_other) noexcept : Patch (_other.data)
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

Patch::Patch (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (block_cast<Handling::Handle<PlainPatch>> (_data));
}

Patch::Patch (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Handling::Handle<PlainPatch> (std::move (block_cast<Handling::Handle<PlainPatch>> (_data)));
}
} // namespace Emergence::StandardLayout
