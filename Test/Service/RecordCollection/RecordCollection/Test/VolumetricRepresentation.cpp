#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/VolumetricRepresentation.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::VolumetricRepresentationTestIncludeMarker () noexcept
{
    return true;
}

void ExecuteVolumetricRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    using BoundingBoxReflection = Emergence::Query::Test::BoundingBox::Reflection;

    tasks.emplace_back (
        CreateVolumetricRepresentation
            {
                "source",
                {
                    {-100.0f, BoundingBoxReflection::minX, 100.0f, BoundingBoxReflection::maxX},
                    {-100.0f, BoundingBoxReflection::minY, 100.0f, BoundingBoxReflection::maxY}
                }
            });

    tasks += ReferenceApiTestImporters::ForRepresentationReference (_scenario, "source");
    tasks.emplace_back (DropRepresentation {"source"});
    Scenario (BoundingBoxReflection::GetMapping (), tasks);
}

BEGIN_SUITE (VolumetricRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteVolumetricRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (VolumetricRepresentationQueries)

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreationAndTestRayIntersections)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::VolumetricQuery::RayIntersections2D ());
}

TEST_CASE (InsertBeforeCreationAndTestShapeIntersections)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::VolumetricQuery::ShapeIntersections2D ());
}

END_SUITE