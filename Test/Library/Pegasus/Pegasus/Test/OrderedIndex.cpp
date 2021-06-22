#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Shortcuts.hpp>
#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

struct EntityIdBound
{
    decltype (Record::entityId) entityId;
};

struct NicknameBound
{
    decltype (Record::nickname) nickname;
};

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

namespace Bounds
{
EntityIdBound entity0 {0u};

EntityIdBound entity1 {1u};

EntityIdBound entity2 {2u};

EntityIdBound entity3 {3u};

NicknameBound hugo {{"hugo"}};

NicknameBound ioran {{"ioran"}};

NicknameBound karl {{"karl"}};

NicknameBound xavier {{"xavier"}};

NicknameBound zanek {{"zanek"}};
};

BOOST_AUTO_TEST_SUITE (OrderedIndex)

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

                    OrderedIndexLookupToRead {{{"entity", "entities[1,2]"}, &Bounds::entity1, &Bounds::entity2}},
                    CursorCheckAllOrdered {"entities[1,2]", {&entity1Karl, &entity2Xavier}},

                    OrderedIndexLookupToRead {{{"entity", "entities[0]"}, &Bounds::entity0, &Bounds::entity0}},
                    CursorCheckAllOrdered {"entities[0]", {&entity0Hugo}},

                    OrderedIndexLookupToReadReversed {{{"entity", "entities[1,2,3]Reversed"},
                                                          &Bounds::entity0, &Bounds::entity2}},
                    CursorCheckAllOrdered {"entities[1,2,3]Reversed", {&entity2Xavier, &entity1Karl, &entity0Hugo}},

                    OrderedIndexLookupToRead {{{"entity", "entities[2,3]"}, &Bounds::entity2, &Bounds::entity3}},
                    CursorCheckAllOrdered {"entities[2,3]", {&entity2Xavier}},

                    OrderedIndexLookupToRead {{{"entity", "entities[3+]"}, &Bounds::entity3, nullptr}},
                    CursorCheck {"entities[3+]", nullptr},

                    OrderedIndexLookupToRead {{{"entity", "entities[-0]"}, nullptr, &Bounds::entity0}},
                    CursorCheckAllOrdered {"entities[-0]", {&entity0Hugo}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    CreateOrderedIndex {"nickname", Record::Reflection::nickname},
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},

                    OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"all", {&entity0Hugo, &entity1Karl, &entity2Xavier}},

                    OrderedIndexLookupToRead {{{"nickname", "ioran-zanek"}, &Bounds::ioran, &Bounds::zanek}},
                    CursorCheckAllOrdered {"ioran-zanek", {&entity1Karl, &entity2Xavier}},

                    OrderedIndexLookupToRead {{{"nickname", "hugo-karl"}, &Bounds::hugo, &Bounds::karl}},
                    CursorCheckAllOrdered {"hugo-karl", {&entity0Hugo, &entity1Karl}},

                    OrderedIndexLookupToRead {{{"nickname", "xavier"}, &Bounds::xavier, &Bounds::xavier}},
                    CursorCheckAllOrdered {"xavier", {&entity2Xavier}},

                    OrderedIndexLookupToReadReversed {{{"nickname", "zanek-karl"}, &Bounds::karl, &Bounds::zanek}},
                    CursorCheckAllOrdered {"zanek-karl", {&entity2Xavier, &entity1Karl}},

                    OrderedIndexLookupToReadReversed {{{"nickname", "xavier-null"}, nullptr, &Bounds::xavier}},
                    CursorCheckAllOrdered {"xavier-null", {&entity2Xavier, &entity1Karl, &entity0Hugo}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    AllocateAndInit {&entity1Karl},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities",
                                           {&entity0Hugo, &entity1Karl, &entity1Karl, &entity1Karl, &entity2Xavier}},

                    OrderedIndexLookupToRead {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
                    CursorCheckAllOrdered {"entity1", {&entity1Karl, &entity1Karl, &entity1Karl}},

                    OrderedIndexLookupToReadReversed {{{"entity", "entity[-1]Reversed"}, nullptr, &Bounds::entity1}},
                    CursorCheckAllOrdered {"entity[-1]Reversed",
                                           {&entity1Karl, &entity1Karl, &entity1Karl, &entity0Hugo}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    AllocateAndInit {&entity1Karl},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities",
                                           {&entity0Hugo, &entity1Karl, &entity1Karl, &entity1Karl, &entity2Xavier}},
                    CloseCursor {"allEntities"},

                    OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorEdit {"entity1", &entity2Xavier},
                    CursorIncrement {"entity1"},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorEdit {"entity1", &entity0Hugo},
                    CloseCursor {"entity1"},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities",
                                           {&entity0Hugo, &entity0Hugo, &entity1Karl, &entity2Xavier, &entity2Xavier}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
                    CloseCursor {"allEntities"},

                    OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorDeleteRecord {"entity1"},
                    CursorCheck {"entity1", nullptr},
                    CloseCursor {"entity1"},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity2Xavier}},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},
                    CreateOrderedIndex {"entity", Record::Reflection::entityId},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
                    CloseCursor {"allEntities"},

                    OrderedIndexLookupToEditReversed {{{"entity", "entity[-1]"}, nullptr, &Bounds::entity1}},
                    CursorCheck {"entity[-1]", &entity1Karl},
                    CursorEdit {"entity[-1]", &entity2Xavier},
                    CursorIncrement {"entity[-1]"},
                    CursorCheck {"entity[-1]", &entity0Hugo},
                    CursorEdit {"entity[-1]", &entity1Karl},
                    CloseCursor {"entity[-1]"},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity1Karl, &entity2Xavier, &entity2Xavier}},
                    CloseCursor {"allEntities"},

                    OrderedIndexLookupToEditReversed {{{"entity", "entity2"}, &Bounds::entity2, &Bounds::entity2}},
                    CursorCheck {"entity2", &entity2Xavier},
                    CursorDeleteRecord {"entity2"},
                    CursorCheck {"entity2", &entity2Xavier},
                    CloseCursor {"entity2"},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity1Karl, &entity2Xavier}},
                    CloseCursor {"allEntities"},
                }
            },
            {
                Record::Reflection::GetMapping (),
                {
                    OpenAllocator {},
                    AllocateAndInit {&entity2Xavier},
                    AllocateAndInit {&entity0Hugo},
                    AllocateAndInit {&entity1Karl},
                    AllocateAndInit {&entity1Karl},
                    CloseAllocator {},

                    CreateOrderedIndex {"entity", Record::Reflection::entityId},
                    CreateOrderedIndex {"nickname", Record::Reflection::nickname},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity1Karl, &entity1Karl, &entity2Xavier}},
                    CloseCursor {"allEntities"},

                    OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"all", {&entity0Hugo, &entity1Karl, &entity1Karl, &entity2Xavier}},
                    CloseCursor {"all"},

                    OrderedIndexLookupToEdit {{{"entity", "entity1"}, &Bounds::entity1, &Bounds::entity1}},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorEdit {"entity1", &entity2Xavier},
                    CursorDeleteRecord {"entity1"},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorEdit {"entity1", &entity2Xavier},
                    CloseCursor {"entity1"},

                    OrderedIndexLookupToRead {{{"entity", "allEntities"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"allEntities", {&entity0Hugo, &entity2Xavier, &entity2Xavier}},

                    OrderedIndexLookupToRead {{{"nickname", "all"}, nullptr, nullptr}},
                    CursorCheckAllOrdered {"all", {&entity0Hugo, &entity2Xavier, &entity2Xavier}},
                }
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()