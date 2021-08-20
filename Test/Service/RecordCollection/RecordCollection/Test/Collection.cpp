#include <RecordCollection/Test/Collection.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::CollectionTestIncludeMarker () noexcept
{
    return true;
}

BEGIN_SUITE (Collection)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations)

TEST_CASE (DropRepresentation)
{
    using namespace Emergence;

    Scenario {
        Query::Test::PlayerWithBoundingBox::Reflection::GetMapping (),
        {
            CreateLinearRepresentation
                {
                    "playerName",
                    StandardLayout::ProjectNestedField (
                        Query::Test::PlayerWithBoundingBox::Reflection::player,
                        Query::Test::Player::Reflection::name)
                },

            CreatePointRepresentation
                {
                    "playerId",
                    {
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::player,
                            Query::Test::Player::Reflection::id),
                    }
                },

            CreatePointRepresentation
                {
                    "playerAlive",
                    {
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::player,
                            Query::Test::Player::Reflection::alive),
                    }
                },

            CreateVolumetricRepresentation {
                "2d",
                {
                    {
                        -100.0f,
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::boundingBox,
                            Query::Test::BoundingBox::Reflection::minX),

                        100.0f,
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::boundingBox,
                            Query::Test::BoundingBox::Reflection::maxX),
                    },
                    {
                        -100.0f,
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::boundingBox,
                            Query::Test::BoundingBox::Reflection::minY),

                        100.0f,
                        StandardLayout::ProjectNestedField (
                            Query::Test::PlayerWithBoundingBox::Reflection::boundingBox,
                            Query::Test::BoundingBox::Reflection::maxY),
                    }
                }},

            OpenAllocator {},
            AllocateAndInit {&Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5},
            AllocateAndInit {&Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2},
            AllocateAndInit {&Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            CloseAllocator {},

            DropRepresentation {"playerId"},
            QueryValueToRead {{{"playerAlive", "alive"}, &Query::Test::Queries::ALIVE}},
            CursorCheckAllUnordered
                {
                    "alive",
                    {
                        &Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5,
                        &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                        &Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60
                    }
                },

            QueryValueToRead {{{"playerAlive", "dead"}, &Query::Test::Queries::DEAD}},
            CursorCheckAllUnordered {"dead", {}},

            QueryAscendingRangeToRead {{{"playerName", "names"}, nullptr, nullptr}},
            CursorCheckAllOrdered
                {
                    "names",
                    {
                        &Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5,
                        &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                        &Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60
                    }
                },

            QueryShapeIntersectionToRead
                {{{"2d", "min = (-3, 0), max = (11, 11)"}, {-3.0f, 0.0f}, {11.0f, 11.0f}}},
            CursorCheckAllUnordered
                {
                    "min = (-3, 0), max = (11, 11)",
                    {
                        &Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5,
                        &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                    }
                },
        }
    };
}

END_SUITE