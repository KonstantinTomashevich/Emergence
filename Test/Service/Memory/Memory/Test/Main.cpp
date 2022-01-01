#include <Memory/Test/Heap.hpp>
#include <Memory/Test/OrderedPool.hpp>
#include <Memory/Test/Stack.hpp>
#include <Memory/Test/UniqueString.hpp>
#include <Memory/Test/UnorderedPool.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool HEAP_TEST_INCLUDE_MARKER = Emergence::Memory::Test::HeapTestIncludeMarker ();
static const bool ORDERED_POOL_TEST_INCLUDE_MARKER = Emergence::Memory::Test::OrderedPoolTestIncludeMarker ();
static const bool STACK_TEST_INCLUDE_MARKER = Emergence::Memory::Test::StackTestIncludeMarker ();
static const bool UNIQUE_STRING_TEST_INCLUDE_MARKER = Emergence::Memory::Test::UniqueStringTestIncludeMarker ();
static const bool UNORDERED_POOL_TEST_INCLUDE_MARKER = Emergence::Memory::Test::UnorderedPoolTestIncludeMarker ();
