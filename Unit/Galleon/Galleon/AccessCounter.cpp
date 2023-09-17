#include <Assert/Assert.hpp>

#include <Galleon/AccessCounter.hpp>

namespace Emergence::Galleon
{
void AccessCounter::RegisterReadAccess () noexcept
{
    // Write access count can not be changed by thread safe operations, therefore it's ok to check it here.
    EMERGENCE_ASSERT (write == 0u || unsafeFetchAllowed);
    ++read;
}

void AccessCounter::UnregisterReadAccess () noexcept
{
    --read;
}

void AccessCounter::RegisterWriteAccess () noexcept
{
    EMERGENCE_ASSERT (read == 0u || unsafeFetchAllowed);
    EMERGENCE_ASSERT (write == 0u);
    ++write;
}

void AccessCounter::UnregisterWriteAccess () noexcept
{
    EMERGENCE_ASSERT (write == 1u);
    --write;
}

void AccessCounter::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    // Unsafe access should be carefully controlled by user, therefore there should be no set-set or unset-unset calls.
    EMERGENCE_ASSERT (unsafeFetchAllowed != _allowed);
    unsafeFetchAllowed = _allowed;
}
} // namespace Emergence::Galleon
