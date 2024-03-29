#include <RecordCollection/Test/Scenario.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

using namespace Emergence::RecordCollection::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return {BoundingBox::Reflect ().mapping,
            {&BOX_MIN_10_8_4_MAX_11_9_5},
            {Sources::Volumetric {"Source",
                                  {{-100.0f, BoundingBox::Reflect ().minX, 100.0f, BoundingBox::Reflect ().maxX},
                                   {-100.0f, BoundingBox::Reflect ().minY, 100.0f, BoundingBox::Reflect ().maxY}}}}};
}

static void ExecuteRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForRepresentationReference (_scenario, GetTestStorage ());
}

static void ExecuteShapeIntersectionReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryShapeIntersectionToRead {{{}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
                                          &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteShapeIntersectionEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryShapeIntersectionToEdit {{{}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
                                          &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteRayIntersectionReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryRayIntersectionToRead {{{}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                          &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteRayIntersectionEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryRayIntersectionToEdit {{{}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                          &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

BEGIN_SUITE (VolumetricRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (VolumetricRepresentationShapeIntersectionReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteShapeIntersectionReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (VolumetricRepresentationShapeIntersectionEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteShapeIntersectionEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteShapeIntersectionEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (VolumetricRepresentationRayIntersectionReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteRayIntersectionReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (VolumetricRepresentationRayIntersectionEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteRayIntersectionEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteRayIntersectionEditCursorReferenceApiTest, MoveChain)

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
