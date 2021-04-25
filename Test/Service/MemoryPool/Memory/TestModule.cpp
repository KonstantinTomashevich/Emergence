#define BOOST_TEST_MODULE MemoryPool

#include <boost/test/unit_test.hpp>

#include <Memory/Test/Pool.hpp>

// Visual Studio Clang ignores translation units with automatically registered tests. Currently, usage of
// /WHOLEARCHIVE linker option leads to strange errors, therefore we use makeshift method to make sure that
// linker will not ignore required translation units.
static const bool poolTestIncludeMarker = Emergence::Memory::Test::PoolTestIncludeMarker ();