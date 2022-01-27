#include <Query/Test/DataTypes.hpp>

#include <Testing/Testing.hpp>

#include <Warehouse/Test/Scenario.hpp>
#include <Warehouse/Test/Visualization.hpp>
#include <Warehouse/Test/VisualizationGraphs.hpp>

bool Emergence::Warehouse::Test::VisualizationTestIncludeMarker () noexcept
{
    return true;
}

using namespace Emergence;
using namespace Emergence::Warehouse::Test;
using namespace Emergence::VisualGraph::Common::Constants;

static void CheckResult (const Scenario::Visualization &_result, const VisualizationExpectedResult &_expected)
{
    CHECK (_result.registry == _expected.registry);
    CHECK_EQUAL (_result.queries.size (), _expected.queries.size ());

    for (std::size_t index = 0u; index < std::min (_result.queries.size (), _expected.queries.size ()); ++index)
    {
        CHECK (_result.queries[index] == _expected.queries[index]);
    }
}

BEGIN_SUITE (Visualization)

TEST_CASE (OneQueryForEachCombination)
{
    Scenario::Visualization result = Scenario {
        {
            PrepareFetchSingletonQuery {{Query::Test::BoundingBox::Reflect ().mapping, "Singleton"}},
            PrepareFetchSequenceQuery {{Query::Test::Player::Reflect ().mapping, "Sequence"}},
            PrepareFetchAscendingRangeQuery {
                {Query::Test::PlayerWithBoundingBox::Reflect ().mapping, "Range"},
                StandardLayout::ProjectNestedField (Query::Test::PlayerWithBoundingBox::Reflect ().player,
                                                    Query::Test::Player::Reflect ().id)},
        }}.ExecuteAndVisualize ();

    CheckResult (result, Graphs::OneQueryForEachCombination ());
}

TEST_CASE (Singletons)
{
    Scenario::Visualization result = Scenario {
        {
            PrepareFetchSingletonQuery {{Query::Test::Player::Reflect ().mapping, "First"}},
            PrepareModifySingletonQuery {{Query::Test::BoundingBox::Reflect ().mapping, "Second"}},
        }}.ExecuteAndVisualize ();

    CheckResult (result, Graphs::Singletons ());
}

TEST_CASE (Sequences)
{
    Scenario::Visualization result = Scenario {
        {
            PrepareInsertShortTermQuery {{Query::Test::Player::Reflect ().mapping, "Insert"}},
            PrepareFetchSequenceQuery {{Query::Test::Player::Reflect ().mapping, "First"}},
            PrepareModifySequenceQuery {{Query::Test::BoundingBox::Reflect ().mapping, "Second"}},
        }}.ExecuteAndVisualize ();

    CheckResult (result, Graphs::Sequences ());
}

TEST_CASE (LongTerms)
{
    const Query::Test::Player::Reflection &playerReflection = Query::Test::Player::Reflect ();
    const Query::Test::BoundingBox::Reflection &boxReflection = Query::Test::BoundingBox::Reflect ();

    Container::Vector<Query::Test::Sources::Volumetric::Dimension> dimensions {
        {-100.0f, boxReflection.minX, 100.0f, boxReflection.maxX},
        {-100.0f, boxReflection.minY, 100.0f, boxReflection.maxY},
        {-100.0f, boxReflection.minZ, 100.0f, boxReflection.maxZ},
    };

    Scenario::Visualization result = Scenario {
        {
            PrepareInsertLongTermQuery {{playerReflection.mapping, "Insert"}},
            PrepareFetchAscendingRangeQuery {{playerReflection.mapping, "FetchId"}, playerReflection.id},
            PrepareModifyAscendingRangeQuery {{playerReflection.mapping, "ModifyId"}, playerReflection.id},

            PrepareFetchDescendingRangeQuery {{playerReflection.mapping, "FetchName"}, playerReflection.name},
            PrepareModifyDescendingRangeQuery {{playerReflection.mapping, "ModifyName"}, playerReflection.name},

            PrepareFetchValueQuery {{playerReflection.mapping, "FetchAlive"}, {playerReflection.alive}},
            PrepareModifyValueQuery {{playerReflection.mapping, "ModifyAlive"}, {playerReflection.alive}},

            PrepareFetchShapeIntersectionQuery {{boxReflection.mapping, "FetchShape"}, dimensions},
            PrepareModifyShapeIntersectionQuery {{boxReflection.mapping, "ModifyShape"}, dimensions},

            PrepareFetchRayIntersectionQuery {{boxReflection.mapping, "FetchRay"}, dimensions},
            PrepareModifyRayIntersectionQuery {{boxReflection.mapping, "ModifyRay"}, dimensions},
        }}.ExecuteAndVisualize ();

    CheckResult (result, Graphs::LongTerms ());
}

END_SUITE
