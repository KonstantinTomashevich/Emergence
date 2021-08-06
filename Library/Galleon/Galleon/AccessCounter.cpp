#include <cassert>

#include <Galleon/AccessCounter.hpp>

namespace Emergence::Galleon
{
AccessCounter::AccessCounter () noexcept
    : read (0u),
      write (0u)
{
}

AccessCounter::AccessCounter (AccessCounter &&_other) noexcept
    : read (_other.read.load ()),
      write (_other.write)
{
    _other.read = 0u;
    _other.write = 0u;
}

void AccessCounter::RegisterReadAccess () noexcept
{
    // Write access count can not be changed by thread safe operations, therefore it's ok to check it here.
    assert (write == 0u);
    ++read;
}

void AccessCounter::UnregisterReadAccess () noexcept
{
    --read;
}

void AccessCounter::RegisterWriteAccess () noexcept
{
    assert (read == 0u);
    assert (write == 0u);
    ++write;
}

void AccessCounter::UnregisterWriteAccess () noexcept
{
    assert (write == 1u);
    --write;
}
} // namespace Emergence::Galleon