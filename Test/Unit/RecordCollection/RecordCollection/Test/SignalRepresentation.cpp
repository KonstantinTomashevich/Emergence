#include <API/Common/BlockCast.hpp>

#include <RecordCollection/Test/Scenario.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/SignalQueryTests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

using namespace Emergence::RecordCollection::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return {
        Player::Reflect ().mapping,
        {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
        {Sources::Signal {"Source", Player::Reflect ().id, array_cast<std::uint32_t, sizeof (std::uint64_t)> (0u)}}};
}

static void ExecuteRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForRepresentationReference (_scenario, GetTestStorage ());
}

static void ExecuteReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (), QuerySignalToRead {},
                                          &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (_scenario, GetTestStorage (), QuerySignalToEdit {},
                                          &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

BEGIN_SUITE (SignalRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (SignalRepresentationReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (SignalRepresentationEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (SignalRepresentationQueries)

REGISTER_ALL_SIGNAL_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::SignalQuery::SimpleLookup ());
}

END_SUITE
