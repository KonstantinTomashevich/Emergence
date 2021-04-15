#define BOOST_TEST_MODULE MappingBuilder

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <StandardLayout/MappingBuilder.hpp>

namespace Tests::MappingBuilder
{
using namespace Emergence::StandardLayout;

static bool operator == (const Mapping &_first, const Mapping &second) noexcept
{
    // There is no better implementation agnostic way of checking mapping equality than to check equality of all fields.
    // TODO: Implement.
    return false;
}
}