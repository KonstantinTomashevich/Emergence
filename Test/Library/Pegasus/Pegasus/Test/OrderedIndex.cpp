#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Shortcuts.hpp>
#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

BOOST_AUTO_TEST_SUITE (OrderedIndex)

static Record entity0Hugo
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

static Record entity1Karl
    {
        1u,
        {
            "karl"
        },
        {
            5u,
            1u,
        },
        {
            25u,
            71.0f,
        },
        {
            5.0f,
            4.0f,
            11.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_POISONED,
    };

static Record entity2Xavier
    {
        2u,
        {
            "xavier"
        },
        {
            3u,
            2u,
        },
        {
            67u,
            65.0f,
        },
        {
            9.0f,
            7.0f,
            82.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

BOOST_DATA_TEST_CASE(
    TestRoutine, boost::unit_test::data::monomorphic::collection (
    std::vector <Scenario>
        {
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateOrderedIndex {"source", Record::Reflection::entityId},
                    } +
                Shortcuts::TestIsCanBeDropped ("source")
            },
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateOrderedIndex {"entity", Record::Reflection::entityId},
                        OpenAllocator {},
                        AllocateAndInit {&entity2Xavier},
                        AllocateAndInit {&entity0Hugo},
                        AllocateAndInit {&entity1Karl},
                        CloseAllocator {},
                    } +
                Shortcuts::TestCursorCopyAndMove (
                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    OrderedIndexLookupToEdit {{{"entity", "allEntities"}, nullptr, nullptr}},
                    &entity0Hugo, &entity1Karl, &entity0Hugo)
            },
            {
                Record::Reflection::GetMapping (),
                std::vector <Task>
                    {
                        CreateOrderedIndex {"entity", Record::Reflection::entityId},
                        OpenAllocator {},
                        AllocateAndInit {&entity2Xavier},
                        AllocateAndInit {&entity0Hugo},
                        AllocateAndInit {&entity1Karl},
                        CloseAllocator {},
                    } +
                Shortcuts::TestCursorCopyAndMove (
                    OrderedIndexLookupToReadReversed {{{"entity", "allEntities"}, nullptr, nullptr}},
                    OrderedIndexLookupToEditReversed {{{"entity", "allEntities"}, nullptr, nullptr}},
                    &entity2Xavier, &entity1Karl, &entity2Xavier)
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},

                    OrderedIndexLookupToReadReversed {{{"entity", "allEntitiesReversed"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntitiesReversed", {&entity2Xavier, &entity1Karl, &entity0Hugo}},
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()