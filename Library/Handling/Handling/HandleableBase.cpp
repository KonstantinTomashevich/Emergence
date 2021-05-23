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

bool HandleableBase::HasAnyReferences () const noexcept
{
    return references > 0u;
}

HandleableBase::HandleableBase ()
    : references (0u)
{
}
} // namespace Emergence::Handling