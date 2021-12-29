#include <Memory/Test/PoolShared.hpp>
#include <Memory/Test/UnorderedPool.hpp>
#include <Memory/UnorderedPool.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Test
{
bool UnorderedPoolTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Test

using namespace Emergence::Memory::Test::Pool;

BEGIN_SUITE (UnorderedPool)

ALL_SHARED_POOL_TESTS (Emergence::Memory::UnorderedPool)

END_SUITE
