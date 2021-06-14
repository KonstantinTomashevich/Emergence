#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

BOOST_AUTO_TEST_SUITE (Storage)

BOOST_DATA_TEST_CASE(
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Scenario>
        {
            // Empty scenario, that checks that storage can be safely created and deleted.
            {
                Record::Reflection::GetMapping (),
                {
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()