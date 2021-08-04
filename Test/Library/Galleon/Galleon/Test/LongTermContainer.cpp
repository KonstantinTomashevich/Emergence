#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/RangeQueryTests.hpp>
#include <Query/Test/ValueQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

// TODO: Test referencing.

BEGIN_SUITE (LongTermContainer)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainer::Range)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainer::Value)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainer::Volumetric)

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDriver)

END_SUITE