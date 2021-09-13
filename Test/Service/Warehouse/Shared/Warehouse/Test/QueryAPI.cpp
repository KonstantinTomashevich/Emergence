#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/RangeQueryTests.hpp>
#include <Query/Test/SingletonQueryTests.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>
#include <Query/Test/ValueQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

#include <Warehouse/Test/QueryAPI.hpp>
#include <Warehouse/Test/Scenario.hpp>

using namespace Emergence::Warehouse::Test;

bool Emergence::Warehouse::Test::QueryAPITestIncludeMarker () noexcept
{
    return true;
}

BEGIN_SUITE (QueryAPIAllParametricQueryTypesInOneStorage)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (QueryAPIRange)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (QueryAPISingleton)

REGISTER_ALL_SINGLETON_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (QueryAPIUnorderedSequence)

REGISTER_ALL_UNORDERED_SEQUENCE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (QueryAPIValue)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (QueryAPIVolumetric)

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDriver)

END_SUITE
