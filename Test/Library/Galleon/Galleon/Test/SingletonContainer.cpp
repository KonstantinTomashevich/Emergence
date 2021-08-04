#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/SingletonQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

BEGIN_SUITE (SingletonContainer)

REGISTER_ALL_SINGLETON_QUERY_TESTS (TestQueryApiDriver)

END_SUITE