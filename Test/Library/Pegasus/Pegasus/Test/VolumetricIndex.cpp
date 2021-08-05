#include <Pegasus/Test/Common.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

BEGIN_SUITE (VolumetricIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Emergence::Query::Test::BoundingBox::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateVolumetricIndex {
                    "2d",
                    {
                        {
                            -100.0f, Emergence::Query::Test::BoundingBox::Reflection::minX,
                            100.0f, Emergence::Query::Test::BoundingBox::Reflection::maxX
                        },
                        {
                            -100.0f, Emergence::Query::Test::BoundingBox::Reflection::minY,
                            100.0f, Emergence::Query::Test::BoundingBox::Reflection::maxY
                        }
                    }},
            } +
        Common::TestIsCanBeDropped ("2d")
    };
}

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreationAndTestRayIntersections)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (
        Emergence::Query::Test::VolumetricQuery::RayIntersections2D ());
}

TEST_CASE (InsertBeforeCreationAndTestShapeIntersections)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (
        Emergence::Query::Test::VolumetricQuery::ShapeIntersections2D ());
}

END_SUITE