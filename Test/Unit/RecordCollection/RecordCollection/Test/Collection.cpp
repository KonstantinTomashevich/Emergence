#include <RecordCollection/Collection.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

BEGIN_SUITE (Collection)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations)

TEST_CASE (DropRepresentation)
{
    using namespace Emergence;

    Scenario {
        Query::Test::PlayerWithBoundingBox::Reflect ().mapping,
        {
            CreateLinearRepresentation {
                "playerName", StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                                  Query::Test::Player::Reflect ().name)},

            CreatePointRepresentation {
                "playerId",
                {
                    StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                        Query::Test::Player::Reflect ().id),
                }},

            CreatePointRepresentation {
                "playerAlive",
                {
                    StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                        Query::Test::Player::Reflect ().alive),
                }},

            CreateVolumetricRepresentation {
                "2d",
                {{
                     -100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().minX),

                     100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().maxX),
                 },
                 {
                     -100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().minY),

                     100.0f,
                     StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().boundingBox,
                                                         Query::Test::BoundingBox::Reflect ().maxY),
                 }}},

            OpenAllocator {},
            AllocateAndInit {&Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5},
            AllocateAndInit {&Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2},
            AllocateAndInit {&Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60},
            CloseAllocator {},

            DropRepresentation {"playerId"},
            QueryValueToRead {{{"playerAlive", "alive"}, &Query::Test::Queries::ALIVE}},
            CursorCheckAllUnordered {
                "alive",
                {&Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5, &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                 &Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60}},

            QueryValueToRead {{{"playerAlive", "dead"}, &Query::Test::Queries::DEAD}},
            CursorCheckAllUnordered {"dead", {}},

            QueryAscendingRangeToRead {{{"playerName", "names"}, nullptr, nullptr}},
            CursorCheckAllOrdered {
                "names",
                {&Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5, &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                 &Query::Test::XAVIER_2_MIN_15_8_50_MAX_19_11_60}},

            QueryShapeIntersectionToRead {{{"2d", "min = (-3, 0), max = (11, 11)"}, {-3.0f, 0.0f}, {11.0f, 11.0f}}},
            CursorCheckAllUnordered {"min = (-3, 0), max = (11, 11)",
                                     {
                                         &Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5,
                                         &Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2,
                                     }},
        }}
        .Execute ();
}

struct alignas (16u) Align16 final
{
    std::array<float, 4u> simdData;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId simdDataBlock;
        std::array<Emergence::StandardLayout::FieldId, 4u> simdData;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect ()
    {
        static Reflection reflection = [] ()
        {
            EMERGENCE_MAPPING_REGISTRATION_BEGIN (Align16);
            EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (simdData);
            EMERGENCE_MAPPING_REGISTRATION_END ();
        }();

        return reflection;
    }
};

TEST_CASE (NonStandardAlignment)
{
    constexpr std::size_t SAMPLE_COUNT = 16u;
    Emergence::RecordCollection::Collection collection (Align16::Reflect ().mapping);
    Emergence::RecordCollection::Collection::Allocator allocator = collection.AllocateAndInsert ();

    for (std::size_t index = 0u; index < SAMPLE_COUNT; ++index)
    {
        CHECK_EQUAL (reinterpret_cast<std::uintptr_t> (allocator.Allocate ()) % alignof (Align16), 0u);
    }
}

END_SUITE
