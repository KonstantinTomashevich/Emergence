#include <Galleon/CargoDeck.hpp>

#include <Query/Test/Data.hpp>

#include <Testing/Testing.hpp>

BEGIN_SUITE (CargoDeck)

// TODO: Check how multiple containers behave in one deck.

TEST_CASE (MoveAssignment)
{
    using namespace Emergence::Galleon;
    using namespace Emergence::Query::Test;

    CargoDeck deck;
    // We test move assignment instead of regular move because it's easier to keep correct destruction order in
    // function scope. Before deck destruction all queries must be destructed, therefore in function scope we can
    // not construct queries after decks.
    CargoDeck otherDeck;

    auto InitSingletonContainer =
        [&deck] ()
        {
            auto handle = deck.AcquireSingletonContainer (Player::Reflection::GetMapping ());
            auto modifyQuery = handle->Modify ();
            auto modificationCursor = modifyQuery.Execute ();

            memcpy (*modificationCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflection::GetMapping ().GetObjectSize ());
            return handle->Fetch ();
        };

    auto InitShortTermContainer =
        [&deck] ()
        {
            auto handle = deck.AcquireShortTermContainer (Player::Reflection::GetMapping ());
            auto insertQuery = handle->Insert ();
            auto insertCursor = insertQuery.Execute ();

            memcpy (++insertCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflection::GetMapping ().GetObjectSize ());
            return handle->Fetch ();
        };

    auto InitLongTermContainer =
        [&deck] ()
        {
            auto handle = deck.AcquireLongTermContainer (Player::Reflection::GetMapping ());
            auto insertQuery = handle->Insert ();

            // RecordCollection as LongTermContainer backend forbids
            // representation creation during other query execution.
            {
                auto insertCursor = insertQuery.Execute ();
                memcpy (++insertCursor, &HUGO_0_ALIVE_STUNNED, Player::Reflection::GetMapping ().GetObjectSize ());
                memcpy (++insertCursor, &KARL_1_ALIVE_IMMOBILIZED, Player::Reflection::GetMapping ().GetObjectSize ());
            }

            return handle->FetchRange (Player::Reflection::id);
        };

    SingletonContainer::FetchQuery singletonFetch = InitSingletonContainer ();
    ShortTermContainer::FetchQuery shortTermFetch = InitShortTermContainer ();
    LongTermContainer::FetchRangeQuery longTermFetchRange = InitLongTermContainer ();

    auto CheckEquality =
        [] (const void *_object, const void *_source)
        {
            return memcmp (_object, _source, Player::Reflection::GetMapping ().GetObjectSize ()) == 0;
        };

    auto CheckQueries =
        [&singletonFetch, &shortTermFetch, &longTermFetchRange, &CheckEquality] ()
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

END_SUITE