#define BOOST_TEST_MODULE Hashing

#include <boost/test/unit_test.hpp>

#include <Hashing/Test/ByteHasher.hpp>

// Visual Studio Clang ignores translation units with automatically registered tests. Currently, usage of
// /WHOLEARCHIVE linker option leads to strange errors, therefore we use makeshift method to make sure that
// linker will not ignore required translation units.
static const bool byteHasherTestIncludeMarker = Emergence::Hashing::Test::ByteHasherTestIncludeMarker ();