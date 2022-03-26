#include <cassert>

#include <Galleon/AccessCounter.hpp>

namespace Emergence::Galleon
{
void AccessCounter::RegisterReadAccess () noexcept
{
    // Write access count can not be changed by thread safe operations, therefore it's ok to check it here.
    assert (write == 0u || unsafeFetchAllowed);
    ++read;
}

void AccessCounter::UnregisterReadAccess () noexcept
{
    --read;
}

void AccessCounter::RegisterWriteAccess () noexcept
{
    assert (read == 0u || unsafeFetchAllowed);
    assert (write == 0u);
    ++write;
}

void AccessCounter::UnregisterWriteAccess () noexcept
{
    assert (write == 1u);
    --write;
}

void AccessCounter::SetUnsafeFetchAllowed (bool _allowed) noexcept
{
    // Unsafe access should be carefully controlled by user, therefore there should be no set-set or unset-unset calls.
    assert (unsafeFetchAllowed != _allowed);
    unsafeFetchAllowed = _allowed;
}
} // namespace Emergence::Galleon
