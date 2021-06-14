#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

struct EntityIdLookupRequest
{
    decltype (Record::entityId) entityId;
};

static Record firstRecord
    {
        0u,
        {
            "hugo"
        },
        {
            1u,
            0u,
        },
        {
            47u,
            96.0f,
        },
        {
            3.0f,
            7.0f,
            49.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_STUNNED,
    };

static Record anotherRecordForEntity0
    {
        0u,
        {
            "duplicate"
        },
        {
            2u,
            1u,
        },
        {
            49u,
            91.0f,
        },
        {
            5.0f,
            1.0f,
            0.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

namespace Requests
{
EntityIdLookupRequest entity0 {0u};
};

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
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    CloseAllocator {},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheck {"entity0", &firstRecord},
                    CursorIncrement {"entity0"},
                    CursorCheck {"entity0", nullptr},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateHashIndex {"entity", {Record::Reflection::entityId}},
                    OpenAllocator {},
                    AllocateAndInit {&firstRecord},
                    AllocateAndInit {&anotherRecordForEntity0},
                    CloseAllocator {},
                    HashIndexLookupToRead {{"entity", "entity0", &Requests::entity0}},
                    CursorCheckAllUnordered {"entity0", {&firstRecord, &anotherRecordForEntity0}},
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()