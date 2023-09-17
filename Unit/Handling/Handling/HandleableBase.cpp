#include <limits>

#include <Assert/Assert.hpp>

#include <Handling/HandleableBase.hpp>

namespace Emergence::Handling
{
void HandleableBase::RegisterReference () noexcept
{
    EMERGENCE_ASSERT (references != std::numeric_limits<decltype (references)::value_type>::max ());
    ++references;
}

void HandleableBase::UnregisterReference () noexcept
{
    EMERGENCE_ASSERT (references > 0u);
    --references;
}

uintptr_t HandleableBase::GetReferenceCount () const noexcept
{
    return references;
}
} // namespace Emergence::Handling
