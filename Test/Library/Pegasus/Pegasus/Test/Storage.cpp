#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <Pegasus/Test/Record.hpp>
#include <Pegasus/Test/Scenario.hpp>

using namespace Emergence::Pegasus::Test;

struct NicknameBound
{
    decltype (Record::nickname) nickname;
};

struct EntityIdLookupRequest
{
    decltype (Record::entityId) entityId;
};

static const float minX = -100.0f;

static const float minY = -100.0f;

static const float maxX = 100.0f;

static const float maxY = 100.0f;

static const float $m3 = -3.0f;

static const float $0 = 0.0f;

static const float $1 = 1.0f;

static const float $2 = 2.0f;

static const float $11 = 11.0f;

static const float $12 = 12.0f;

static const float $20 = 20.0f;

static const std::vector <DimensionDescriptor> dimensions2d =
    {
        {
            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::minX),
            &minX,

            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::maxX),
            &maxX,
        },
        {
            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::minY),
            &minY,

            Emergence::StandardLayout::ProjectNestedField (
                Record::Reflection::boundingBox, BoundingBox::Reflection::maxY),
            &maxY,
        }
    };

static const Record entity0Hugo
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
            10.0f,
            8.0f,
            4.0f,
            11.0f,
            9.0f,
            5.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_STUNNED,
    };

static const Record entity1Karl
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
            -2.0f,
            1.0f,
            0.0f,
            0.0f,
            4.0f,
            2.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_POISONED,
    };

static const Record entity2Xavier
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
            15.0f,
            8.0f,
            50.0f,
            19.0f,
            11.0f,
            60.0f,
        },
        Record::Status::FLAG_ALIVE | Record::Status::FLAG_IMMOBILIZED,
    };

namespace Requests
{
static const EntityIdLookupRequest entity0 {0u};

static const EntityIdLookupRequest entity1 {1u};

static const EntityIdLookupRequest entity2 {2u};
}

namespace Bounds
{
static const NicknameBound karl {{"karl"}};
}

// This module checks that indices of different types work correctly after edition by cursors from other
// indices. Therefore these tests are split into "setup", "edit" and "finalize" parts. Setup part configures
// indices and checks their initial state. Edit part changes ::entity1Karl into ::entity0Hugo, deletes it and
// replaces ::entity2Xavier data with ::entity1Karl data using required index. Finalize part checks index states
// after edition. Setup and finalize parts are shared between all tests below.
static const std::vector <Task> setup =
    {
        OpenAllocator {},
        AllocateAndInit {&entity0Hugo},
        AllocateAndInit {&entity1Karl},
        AllocateAndInit {&entity2Xavier},
        CloseAllocator {},

        CreateHashIndex {"entity", {Record::Reflection::entityId}},
        CreateOrderedIndex {"nickname", Record::Reflection::nickname},
        CreateVolumetricIndex {"2d", dimensions2d},

        HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
        CursorCheckAllUnordered {"entity0", {&entity0Hugo}},
        CloseCursor {"entity0"},

        HashIndexLookupToRead {{{"entity", "entity1"}, &Requests::entity1}},
        CursorCheckAllUnordered {"entity1", {&entity1Karl}},
        CloseCursor {"entity1"},

        HashIndexLookupToRead {{{"entity", "entity2"}, &Requests::entity2}},
        CursorCheckAllUnordered {"entity2", {&entity2Xavier}},
        CloseCursor {"entity2"},

        OrderedIndexLookupToRead {{{"nickname", "nicknames"}, nullptr, nullptr}},
        CursorCheckAllOrdered {"nicknames", {&entity0Hugo, &entity1Karl, &entity2Xavier}},
        CloseCursor {"nicknames"},

        VolumetricIndexShapeIntersectionLookupToRead
            {
                {
                    {"2d", "x = [-3, 11], y = [0, 11]"},
                    {&$m3, &$0}, {&$11, &$11}
                }
            },
        CursorCheckAllUnordered {"x = [-3, 11], y = [0, 11]", {&entity0Hugo, &entity1Karl}},
        CloseCursor {"x = [-3, 11], y = [0, 11]"},

        VolumetricIndexShapeIntersectionLookupToRead
            {
                {
                    {"2d", "x = [12, 20], y = [0, 11]"},
                    {&$12, &$0}, {&$20, &$11}
                }
            },
        CursorCheckAllUnordered {"x = [12, 20], y = [0, 11]", {&entity2Xavier}},
        CloseCursor {"x = [12, 20], y = [0, 11]"},
    };

static const std::vector <Task> finalize =
    {
        HashIndexLookupToRead {{{"entity", "entity0"}, &Requests::entity0}},
        CursorCheckAllUnordered {"entity0", {&entity0Hugo}},

        HashIndexLookupToRead {{{"entity", "entity1"}, &Requests::entity1}},
        CursorCheckAllUnordered {"entity1", {&entity1Karl}},

        HashIndexLookupToRead {{{"entity", "entity2"}, &Requests::entity2}},
        CursorCheckAllUnordered {"entity2", {}},

        OrderedIndexLookupToRead {{{"nickname", "nicknames"}, nullptr, nullptr}},
        CursorCheckAllOrdered {"nicknames", {&entity0Hugo, &entity1Karl}},

        VolumetricIndexShapeIntersectionLookupToRead
            {
                {
                    {"2d", "x = [-3, 11], y = [0, 11]"},
                    {&$m3, &$0}, {&$11, &$11}
                }
            },
        CursorCheckAllUnordered {"x = [-3, 11], y = [0, 11]", {&entity0Hugo, &entity1Karl}},

        VolumetricIndexShapeIntersectionLookupToRead
            {
                {
                    {"2d", "x = [12, 20], y = [0, 11]"},
                    {&$12, &$0}, {&$20, &$11}
                }
            },
        CursorCheckAllUnordered {"x = [12, 20], y = [0, 11]", {}},
    };

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
            {
                Record::Reflection::GetMapping (),
                setup +
                std::vector <Task> {
                    HashIndexLookupToEdit {{{"entity", "entity1"}, &Requests::entity1}},
                    CursorCheck {"entity1", &entity1Karl},
                    CursorEdit {"entity1", &entity0Hugo},
                    CursorDeleteRecord {"entity1"},
                    CloseCursor {"entity1"},

                    HashIndexLookupToEdit {{{"entity", "entity2"}, &Requests::entity2}},
                    CursorCheck {"entity2", &entity2Xavier},
                    CursorEdit {"entity2", &entity1Karl},
                    CloseCursor {"entity2"},
                } +
                finalize
            },
            {
                Record::Reflection::GetMapping (),
                setup +
                std::vector <Task> {
                    OrderedIndexLookupToEdit {{{"nickname", "karl_and_xavier"}, &Bounds::karl, nullptr}},
                    CursorCheck {"karl_and_xavier", &entity1Karl},
                    CursorEdit {"karl_and_xavier", &entity0Hugo},
                    CursorDeleteRecord {"karl_and_xavier"},
                    CursorCheck {"karl_and_xavier", &entity2Xavier},
                    CursorEdit {"karl_and_xavier", &entity1Karl},
                    CloseCursor {"karl_and_xavier"},
                } +
                finalize
            },
            {
                Record::Reflection::GetMapping (),
                setup +
                std::vector <Task> {
                    VolumetricIndexRayIntersectionLookupToEdit
                        {
                            {
                                {"2d", "origin = (-3, 0), direction = (2, 1)"},
                                {&$m3, &$0}, {&$2, &$1}
                            }
                        },
                    CursorCheck {"origin = (-3, 0), direction = (2, 1)", &entity1Karl},
                    CursorEdit {"origin = (-3, 0), direction = (2, 1)", &entity0Hugo},
                    CursorDeleteRecord {"origin = (-3, 0), direction = (2, 1)"},
                    CursorCheck {"origin = (-3, 0), direction = (2, 1)", &entity2Xavier},
                    CursorEdit {"origin = (-3, 0), direction = (2, 1)", &entity1Karl},
                    CloseCursor {"origin = (-3, 0), direction = (2, 1)"},
                } +
                finalize
            },
        }))
{
    sample.Execute ();
}

BOOST_AUTO_TEST_SUITE_END ()