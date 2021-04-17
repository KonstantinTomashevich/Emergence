#define BOOST_TEST_MODULE StandardLayoutMapping

#include <boost/test/unit_test.hpp>

#include <StandardLayout/Test/MappingBuilder.hpp>

// Visual Studio Clang ignores translation units with automatically registered tests. Currently, usage of
// /WHOLEARCHIVE linker option leads to strange errors, therefore we use makeshift method to make sure that
// linker will not ignore required translation units.
static const bool MappingBuilderTestIncludeMarker = Emergence::StandardLayout::Test::MappingBuilderTestIncludeMarker ();