#include <RecordCollection/Test/Scenario.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

using namespace Emergence::RecordCollection::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return {Player::Reflect ().mapping,
            {&HUGO_0_KNIGHT_ALIVE_STUNNED},
            {Sources::Value {"Source", {Player::Reflect ().id}}}};
}

static void ExecuteRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForRepresentationReference (_scenario, GetTestStorage ());
}

static void ExecuteReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryValueToRead {{{}, &Emergence::Query::Test::Queries::ID_0}},
                                          &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (),
                                          QueryValueToEdit {{{}, &Emergence::Query::Test::Queries::ID_0}},
                                          &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
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
