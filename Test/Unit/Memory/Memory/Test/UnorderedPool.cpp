#include <Memory/Test/PoolShared.hpp>
#include <Memory/UnorderedPool.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Test::Pool;

BEGIN_SUITE (UnorderedPool)

ALL_SHARED_POOL_TESTS (Emergence::Memory::UnorderedPool)

END_SUITE
