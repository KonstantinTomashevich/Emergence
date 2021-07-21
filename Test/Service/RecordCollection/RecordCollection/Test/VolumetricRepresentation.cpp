#include <RecordCollection/Test/Common.hpp>
#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/VolumetricRepresentation.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::VolumetricRepresentationTestIncludeMarker () noexcept
{
    return true;
}

BEGIN_SUITE (VolumetricRepresentation)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Emergence::Query::Test::BoundingBox::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateVolumetricRepresentation {
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
        Shortcuts::TestIsCanBeDropped ("2d")
    };
}

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