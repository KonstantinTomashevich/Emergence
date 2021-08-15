#include <RecordCollection/Test/LinearRepresentation.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::LinearRepresentationTestIncludeMarker () noexcept
{
    return true;
}

static const char *const TEST_REPRESENTATION_NAME = "Source";

static void ExecuteReferenceApiTest (const std::vector <Task> &_importedScenario)
{
    // TODO: Pack tests using QueryAPI storages same way as it's done in Galleon?
    std::vector <Task> tasks
        {
            CreateLinearRepresentation {TEST_REPRESENTATION_NAME, Emergence::Query::Test::Player::Reflection::id},
            OpenAllocator {},
            AllocateAndInit {&Emergence::Query::Test::HUGO_0_ALIVE_STUNNED},
            AllocateAndInit {&Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED},
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
        _scenario, TEST_REPRESENTATION_NAME, QueryRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteReversedReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryReversedRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED));
}

static void ExecuteReversedEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryReversedRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED));
}

BEGIN_SUITE (LinearRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (LinearRepresentationReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LinearRepresentationEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LinearRepresentationReversedReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReversedReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LinearRepresentationReversedEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteReversedEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteReversedEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LinearRepresentationQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE