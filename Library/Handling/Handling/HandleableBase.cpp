#include <cassert>

#include <Handling/HandleableBase.hpp>

namespace Emergence::Handling
{
void HandleableBase::RegisterReference () noexcept
{
    assert (references + 1u > references);
    ++references;
}

void HandleableBase::UnregisterReference () noexcept
{
    assert (references - 1u < references);
    --references;
}

uintptr_t HandleableBase::GetReferenceCount () const noexcept
{
    return references;
}

HandleableBase::HandleableBase ()
    : references (0u)
{
}
} // namespace Emergence::Handling