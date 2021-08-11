#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

void ExecuteVolumetricIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    using BoundingBoxReflection = Emergence::Query::Test::BoundingBox::Reflection;

    tasks.emplace_back (
        CreateVolumetricIndex
            {
                "source",
                {
                    {-100.0f, BoundingBoxReflection::minX, 100.0f, BoundingBoxReflection::maxX},
                    {-100.0f, BoundingBoxReflection::minY, 100.0f, BoundingBoxReflection::maxY}
                }
            });

    tasks += ReferenceApiTestImporters::ForIndexReference (_scenario, "source");
    tasks.emplace_back (DropIndex {"source"});
    Scenario (BoundingBoxReflection::GetMapping (), tasks);
}

BEGIN_SUITE (VolumetricIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteVolumetricIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (VolumetricIndexQueries)

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