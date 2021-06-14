#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

BOOST_AUTO_TEST_SUITE (HashIndex)

BOOST_DATA_TEST_CASE(
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Scenario>
        {
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"source", {Record::Reflection::entityId}},
                    CheckIndexCanBeDropped {"source", true},
                    CopyIndexReference {"source", "duplicate"},
                    CheckIndexCanBeDropped {"source", false},
                    CheckIndexCanBeDropped {"duplicate", false},
                    RemoveIndexReference {"source"},
                    CheckIndexCanBeDropped {"duplicate", true},
                    DropIndex {"duplicate"},
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()