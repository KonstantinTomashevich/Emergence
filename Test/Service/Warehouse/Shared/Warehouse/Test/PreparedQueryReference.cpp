#include <Query/Test/Data.hpp>

#include <Reference/Test/Tests.hpp>
#include <Testing/Testing.hpp>

#include <Warehouse/Test/Data.hpp>
#include <Warehouse/Test/PreparedQueryReference.hpp>
#include <Warehouse/Test/Scenario.hpp>

using namespace Emergence::Warehouse::Test;

bool Emergence::Warehouse::Test::PreparedQueryReferenceTestIncludeMarker () noexcept
{
    return true;
}

using Player = Emergence::Query::Test::Player;

static void ExecuteFetchSingletonQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario,
                                               PrepareFetchSingletonQuery {{TestSingleton::Reflect ().mapping, {}}},
                                               &TestSingleton::NON_DEFAULT_INSTANCE);
}

static void ExecuteModifySingletonQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario,
                                               PrepareModifySingletonQuery {{TestSingleton::Reflect ().mapping, {}}},
                                               &TestSingleton::NON_DEFAULT_INSTANCE);
}

static void ExecuteInsertShortTermQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario,
                                               PrepareInsertShortTermQuery {{Player::Reflect ().mapping, {}}},
                                               &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteFetchSequenceQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, PrepareFetchSequenceQuery {{Player::Reflect ().mapping, {}}},
                                               &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteModifySequenceQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, PrepareModifySequenceQuery {{Player::Reflect ().mapping, {}}},
                                               &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteInsertLongTermQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, PrepareInsertLongTermQuery {{Player::Reflect ().mapping, {}}},
                                               &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteFetchValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareFetchValueQuery {{Player::Reflect ().mapping, {}}, {Player::Reflect ().id}},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteModifyValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareModifyValueQuery {{Player::Reflect ().mapping, {}}, {Player::Reflect ().id}},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteFetchAscendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareFetchAscendingRangeQuery {{Player::Reflect ().mapping, {}}, Player::Reflect ().id},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteModifyAscendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareModifyAscendingRangeQuery {{Player::Reflect ().mapping, {}}, Player::Reflect ().id},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteFetchDescendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareFetchDescendingRangeQuery {{Player::Reflect ().mapping, {}}, Player::Reflect ().id},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

static void ExecuteModifyDescendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareModifyDescendingRangeQuery {{Player::Reflect ().mapping, {}}, Player::Reflect ().id},
        &Emergence::Query::Test::HUGO_0_KNIGHT_ALIVE_STUNNED);
}

using BoundingBox = Emergence::Query::Test::BoundingBox;

static std::vector<Emergence::Query::Test::Sources::Volumetric::Dimension> GetTestDimensions ()
{
    return {{-100.0f, BoundingBox::Reflect ().minX, 100.0f, BoundingBox::Reflect ().maxX},
            {-100.0f, BoundingBox::Reflect ().minY, 100.0f, BoundingBox::Reflect ().maxY}};
}

static void ExecuteFetchShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareFetchShapeIntersectionQuery {{BoundingBox::Reflect ().mapping, {}}, GetTestDimensions ()},
        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareModifyShapeIntersectionQuery {{BoundingBox::Reflect ().mapping, {}}, GetTestDimensions ()},
        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareFetchRayIntersectionQuery {{BoundingBox::Reflect ().mapping, {}}, GetTestDimensions ()},
        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (
        _scenario, PrepareModifyRayIntersectionQuery {{BoundingBox::Reflect ().mapping, {}}, GetTestDimensions ()},
        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

BEGIN_SUITE (FetchSingletonQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchSingletonQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifySingletonQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifySingletonQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (InsertShortTermQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteInsertShortTermQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchSequenceQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchSequenceQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifySequenceQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifySequenceQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (InsertLongTermQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteInsertLongTermQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchValueQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchValueQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyValueQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyValueQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchAscendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchAscendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyAscendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyAscendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchDescendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchDescendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyDescendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyDescendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchShapeIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchShapeIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyShapeIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyShapeIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (FetchRayIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchRayIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyRayIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyRayIntersectionQueryReferenceApiTest)

END_SUITE
