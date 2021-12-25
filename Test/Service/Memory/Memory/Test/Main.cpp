#include <Memory/Test/OrderedPool.hpp>
#include <Memory/Test/UnorderedPool.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool UNORDERED_POOL_TEST_INCLUDE_MARKER = Emergence::Memory::Test::UnorderedPoolTestIncludeMarker ();
static const bool ORDERED_POOL_TEST_INCLUDE_MARKER = Emergence::Memory::Test::OrderedPoolTestIncludeMarker ();
