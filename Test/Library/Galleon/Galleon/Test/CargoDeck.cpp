#include <Galleon/CargoDeck.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/SingletonQueryTests.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

BEGIN_SUITE (CargoDeck)

TEST_CASE (MoveAssignment)
{
    using namespace Emergence::Galleon;
    using namespace Emergence::Query::Test;

    CargoDeck deck;
    // We test move assignment instead of regular move because it's easier to keep correct destruction order in
    // function scope. Before deck destruction all queries must be destructed, therefore in function scope we can
    // not construct queries after decks.
    CargoDeck otherDeck;

    auto InitSingletonContainer = [&deck] ()
    {
        auto handle = deck.AcquireSingletonContainer (Player::Reflect ().mapping);
        auto modifyQuery = handle->Modify ();
        auto modificationCursor = modifyQuery.Execute ();

        memcpy (*modificationCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflect ().mapping.GetObjectSize ());
        return handle->Fetch ();
    };

    auto InitShortTermContainer = [&deck] ()
    {
        auto handle = deck.AcquireShortTermContainer (Player::Reflect ().mapping);
        auto insertQuery = handle->Insert ();
        auto insertCursor = insertQuery.Execute ();

        memcpy (++insertCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflect ().mapping.GetObjectSize ());
        return handle->Fetch ();
    };

    auto InitLongTermContainer = [&deck] ()
    {
        auto handle = deck.AcquireLongTermContainer (Player::Reflect ().mapping);
        auto insertQuery = handle->Insert ();

        // RecordCollection as LongTermContainer backend forbids
        // representation creation during other query execution.
        {
            auto insertCursor = insertQuery.Execute ();
            memcpy (++insertCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflect ().mapping.GetObjectSize ());
            memcpy (++insertCursor, &KARL_1_ALIVE_IMMOBILIZED, Player::Reflect ().mapping.GetObjectSize ());
        }

        return handle->FetchAscendingRange (Player::Reflect ().id);
    };

    SingletonContainer::FetchQuery singletonFetch = InitSingletonContainer ();
    ShortTermContainer::FetchQuery shortTermFetch = InitShortTermContainer ();
    LongTermContainer::FetchAscendingRangeQuery longTermFetchRange = InitLongTermContainer ();

    auto CheckEquality = [] (const void *_object, const void *_source)
    {
        return memcmp (_object, _source, Player::Reflect ().mapping.GetObjectSize ()) == 0;
    };

    auto CheckQueries = [&singletonFetch, &shortTermFetch, &longTermFetchRange, &CheckEquality] ()
    {
        auto singletonCursor = singletonFetch.Execute ();
        CHECK (CheckEquality (*singletonCursor, &HUGO_0_ALIVE_STUNNED));

        auto shortTermCursor = shortTermFetch.Execute ();
        CHECK (CheckEquality (*shortTermCursor, &HUGO_0_ALIVE_STUNNED));
        ++shortTermCursor;
        CHECK (*shortTermCursor == nullptr);

        auto longTermCursor = longTermFetchRange.Execute (nullptr, nullptr);
        CHECK (CheckEquality (*longTermCursor, &HUGO_0_ALIVE_STUNNED));
        ++longTermCursor;

        CHECK (CheckEquality (*longTermCursor, &KARL_1_ALIVE_IMMOBILIZED));
        ++longTermCursor;
        CHECK (*longTermCursor == nullptr);
    };

    CheckQueries ();
    otherDeck = std::move (deck);
    CheckQueries ();
}

TEST_CASE (ManyContainers)
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::Query::Test::Tasks;
    Scenario scenario;

    auto Append = [&scenario] (const Scenario &_other)
    {
        for (const Storage &storage : _other.storages)
        {
            scenario.storages.emplace_back (storage);
        }

        scenario.tasks += _other.tasks;
    };

    Append (RemapSources (SingletonQuery::EditAndRead (), {{"singleton", "firstSingleton"}}));

    Append ({{
             {BoundingBox::Reflect ().mapping, {&BOX_MIN_M2_1_0_MAX_0_4_2}, {Sources::Singleton {"secondSingleton"}}},
             },
             {
                 QuerySingletonToRead {{"secondSingleton", "secondSingleton"}},
                 CursorCheck {"secondSingleton", &BOX_MIN_M2_1_0_MAX_0_4_2},
                 CursorClose {"secondSingleton"},
             }});

    Append (RemapSources (UnorderedSequenceQuery::EditAndDelete (), {{"sequence", "firstSequence"}}));

    Append ({{
                 {BoundingBox::Reflect ().mapping,
                  {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_M2_1_0_MAX_0_4_2},
                  {Sources::UnorderedSequence {"secondSequence"}}},
             },
             {
                 QueryUnorderedSequenceToRead {{"secondSequence", "values"}},
                 CursorCheckAllUnordered {"values", {&BOX_MIN_10_8_4_MAX_11_9_5, &BOX_MIN_M2_1_0_MAX_0_4_2}},
                 CursorClose {"values"},
             }});

    Append (RemapSources (VolumetricQuery::Edition (), {{"2d", "volumetric2d"}}));

    Append (RemapSources (AllParametricQueryTypesInOneStorage::EditAndDeleteUsingRangeQuery (),
                          {{"playerId", "allPlayerId"}, {"playerName", "allPlayerName"}, {"2d", "all2D"}}));

    Emergence::Galleon::Test::TestQueryApiDriver (scenario);
}

END_SUITE
