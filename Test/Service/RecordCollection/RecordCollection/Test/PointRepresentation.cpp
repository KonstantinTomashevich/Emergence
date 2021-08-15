#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/PointRepresentation.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::PointRepresentationTestIncludeMarker () noexcept
{
    return true;
}

static const char *const TEST_REPRESENTATION_NAME = "Source";

static void ExecuteReferenceApiTest (const std::vector <Task> &_importedScenario)
{
    std::vector <Task> tasks
        {
            CreatePointRepresentation {TEST_REPRESENTATION_NAME, {Emergence::Query::Test::Player::Reflection::id}},
            OpenAllocator {},
            AllocateAndInit {&Emergence::Query::Test::HUGO_0_ALIVE_STUNNED},
            CloseAllocator {},
        };

    tasks += _importedScenario;
    tasks.emplace_back (DropRepresentation {TEST_REPRESENTATION_NAME});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

static void ExecuteRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (
        ReferenceApiTestImporters::ForRepresentationReference (_scenario, TEST_REPRESENTATION_NAME));
}

static void ExecuteReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryValueToRead {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryValueToEdit {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

BEGIN_SUITE (PointRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (PointRepresentationReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (PointRepresentationEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (PointRepresentationQueries)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE