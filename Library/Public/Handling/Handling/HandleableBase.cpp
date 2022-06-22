#include <cassert>
#include <numeric>

#include <Handling/HandleableBase.hpp>

namespace Emergence::Handling
{
void HandleableBase::RegisterReference () noexcept
{
    assert (references != std::numeric_limits<decltype(references)::value_type>::max());
    ++references;
}

void HandleableBase::UnregisterReference () noexcept
{
    assert (references > 0u);
    --references;
}

uintptr_t HandleableBase::GetReferenceCount () const noexcept
{
    return references;
}
} // namespace Emergence::Handling
