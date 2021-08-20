#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>

namespace Emergence::Query::Test::AllParametricQueryTypesInOneStorage
{
using namespace Tasks;

Storage RequestStorage ()
{
    StandardLayout::Mapping mapping = PlayerWithBoundingBox::Reflection::GetMapping ();
    return
        {
            mapping,
            {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2, &XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            {
                Sources::Value
                    {
                        "playerId",
                        {
                            StandardLayout::ProjectNestedField (
                                PlayerWithBoundingBox::Reflection::player, Player::Reflection::id),
                        }
                    },
                Sources::Range
                    {
                        "playerName",
                        StandardLayout::ProjectNestedField (
                            PlayerWithBoundingBox::Reflection::player, Player::Reflection::name),
                    },
                Sources::Volumetric
                    {
                        "2d",
                        {
                            {
                                -100.0f,
                                StandardLayout::ProjectNestedField (
                                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::minX),
                                100.0f,
                                StandardLayout::ProjectNestedField (
                                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::maxX)
                            },
                            {
                                -100.0f,
                                StandardLayout::ProjectNestedField (
                                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::minY),
                                100.0f,
                                StandardLayout::ProjectNestedField (
                                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::maxY)
                            }
                        }
                    }
            }
        };
}

std::vector <Task> PreConditions ()
{
    return
        {
            QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
            CursorCheckAllUnordered {"0", {&HUGO_0_MIN_10_8_4_MAX_11_9_5}},
            CursorClose {"0"},

            QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
            CursorCheckAllUnordered {"1", {&KARL_1_MIN_M2_1_0_MAX_0_4_2}},
            CursorClose {"1"},

            QueryValueToRead {{{"playerId", "2"}, &Queries::ID_2}},
            CursorCheckAllUnordered {"2", {&XAVIER_2_MIN_15_8_50_MAX_19_11_60}},
            CursorClose {"2"},

            QueryAscendingRangeToRead {{{"playerName", "names"}, nullptr, nullptr}},
            CursorCheckAllOrdered
                {
                    "names",
                    {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2, &XAVIER_2_MIN_15_8_50_MAX_19_11_60}
                },
            CursorClose {"names"},

            QueryShapeIntersectionToRead
                {{{"2d", "min = (-3, 0), max = (11, 11)"}, {-3.0f, 0.0f}, {11.0f, 11.0f}}},
            CursorCheckAllUnordered
                {
                    "min = (-3, 0), max = (11, 11)",
                    {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}
                },
            CursorClose {"min = (-3, 0), max = (11, 11)"},

            QueryShapeIntersectionToRead
                {{{"2d", "min = (12, 0), max = (20, 11)"}, {12.0f, 0.0f}, {20.0f, 11.0f}}},
            CursorCheckAllUnordered {"min = (12, 0), max = (20, 11)", {&XAVIER_2_MIN_15_8_50_MAX_19_11_60}},
            CursorClose {"min = (12, 0), max = (20, 11)"},
        };
}

std::vector <Task> PostConditions ()
{
    return
        {
            QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
            CursorCheckAllUnordered {"0", {&HUGO_0_MIN_10_8_4_MAX_11_9_5}},

            QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
            CursorCheckAllUnordered {"1", {&KARL_1_MIN_M2_1_0_MAX_0_4_2}},

            QueryValueToRead {{{"playerId", "2"}, &Queries::ID_2}},
            CursorCheckAllUnordered {"2", {}},

            QueryAscendingRangeToRead {{{"playerName", "names"}, nullptr, nullptr}},
            CursorCheckAllOrdered {"names", {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}},

            QueryShapeIntersectionToRead
                {{{"2d", "min = (-3, 0), max = (11, 11)"}, {-3.0f, 0.0f}, {11.0f, 11.0f}}},
            CursorCheckAllUnordered {
                "min = (-3, 0), max = (11, 11)", {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}},

            QueryShapeIntersectionToRead
                {{{"2d", "min = (12, 0), max = (20, 11)"}, {12.0f, 0.0f}, {20.0f, 11.0f}}},
            CursorCheckAllUnordered {"min = (12, 0), max = (20, 11)", {}},
        };
}

Scenario EditAndDeleteUsingValueQuery ()
{
    Scenario scenario
        {
            {RequestStorage ()},
            PreConditions ()
        };

    scenario.tasks +=
        {
            QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
            CursorCheck {"1", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorEdit {"1", &HUGO_0_MIN_10_8_4_MAX_11_9_5},
            CursorDeleteObject {"1"},
            CursorClose {"1"},

            QueryValueToEdit {{{"playerId", "2"}, &Queries::ID_2}},
            CursorCheck {"2", &XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            CursorEdit {"2", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorClose {"2"},
        };

    scenario.tasks += PostConditions ();
    return scenario;
}

Scenario EditAndDeleteUsingRangeQuery ()
{
    Scenario scenario
        {
            {RequestStorage ()},
            PreConditions ()
        };

    scenario.tasks +=
        {
            QueryAscendingRangeToEdit {{{"playerName", "karl_and_xavier"}, &Queries::KARL, nullptr}},
            CursorCheck {"karl_and_xavier", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorEdit {"karl_and_xavier", &HUGO_0_MIN_10_8_4_MAX_11_9_5},
            CursorDeleteObject {"karl_and_xavier"},
            CursorCheck {"karl_and_xavier", &XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            CursorEdit {"karl_and_xavier", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorClose {"karl_and_xavier"},
        };

    scenario.tasks += PostConditions ();
    return scenario;
}

Scenario EditAndDeleteUsingVolumetricQuery ()
{
    Scenario scenario
        {
            {RequestStorage ()},
            PreConditions ()
        };

    scenario.tasks +=
        {
            QueryRayIntersectionToEdit
                {{{"2d", "origin = (-3, 0), direction = (2, 1)"}, {-3.0f, 0.0f}, {2.0f, 1.0f}}},
            CursorCheck {"origin = (-3, 0), direction = (2, 1)", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorEdit {"origin = (-3, 0), direction = (2, 1)", &HUGO_0_MIN_10_8_4_MAX_11_9_5},
            CursorDeleteObject {"origin = (-3, 0), direction = (2, 1)"},
            CursorCheck {"origin = (-3, 0), direction = (2, 1)", &XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            CursorEdit {"origin = (-3, 0), direction = (2, 1)", &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            CursorClose {"origin = (-3, 0), direction = (2, 1)"},
        };

    scenario.tasks += PostConditions ();
    return scenario;
}
} // namespace Emergence::Query::Test::AllParametricQueryTypesInOneStorage