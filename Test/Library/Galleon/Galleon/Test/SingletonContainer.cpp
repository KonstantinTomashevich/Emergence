#include <Galleon/CargoDeck.hpp>

#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/SingletonQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

BEGIN_SUITE (SingletonContainer)

REGISTER_ALL_SINGLETON_QUERY_TESTS (TestQueryApiDriver)

/// \brief Helper for reference manipulation tests, that initializes container.
Emergence::Handling::Handle <Emergence::Galleon::SingletonContainer> InitContainer (
    Emergence::Galleon::CargoDeck &_deck)
{
    using namespace Emergence::Query::Test;
    auto container = _deck.AcquireSingletonContainer (Player::Reflection::GetMapping ());

    auto modifyQuery = container->Modify ();
    memcpy (*modifyQuery.Execute (), &HUGO_0_ALIVE_STUNNED, Player::Reflection::GetMapping ().GetObjectSize ());
    return container;
}

/// \brief Helper for reference manipulation tests, that checks is container initialized.
bool IsContainerInitialized (Emergence::Galleon::CargoDeck &_deck)
{
    using namespace Emergence::Query::Test;
    auto container = _deck.AcquireSingletonContainer (Player::Reflection::GetMapping ());

    // Prepared query is created on the fly, therefore it will not interfere with other references.
    auto fetchQuery = container->Fetch ();
    return memcmp (*fetchQuery.Execute (), &HUGO_0_ALIVE_STUNNED,
                   Player::Reflection::GetMapping ().GetObjectSize ()) == 0;
}

TEST_CASE (ContainerReferenceManipulation)
{
    Emergence::Galleon::CargoDeck deck;
    CHECK (!IsContainerInitialized (deck));

    auto originalReference = InitContainer (deck);
    CHECK (IsContainerInitialized (deck));

    auto movedReference {std::move (originalReference)};
    CHECK (IsContainerInitialized (deck));

    auto copiedReference {movedReference};
    CHECK (IsContainerInitialized (deck));

    movedReference = nullptr;
    CHECK (IsContainerInitialized (deck));

    auto reacquiredReference = deck.AcquireSingletonContainer (
        Emergence::Query::Test::Player::Reflection::GetMapping ());
    copiedReference = nullptr;
    CHECK (IsContainerInitialized (deck));

    reacquiredReference = nullptr;
    CHECK (!IsContainerInitialized (deck));
}

#define QUERY_REFERENCE_MANIPULATION_TEST(QueryName)                                                                   \
TEST_CASE (QueryName ## QueryReferenceManipulation)                                                                    \
{                                                                                                                      \
    using namespace Emergence::Galleon;                                                                                \
    CargoDeck deck;                                                                                                    \
    CHECK (!IsContainerInitialized (deck));                                                                            \
                                                                                                                       \
    auto originalReference = InitContainer (deck);                                                                     \
    CHECK (IsContainerInitialized (deck));                                                                             \
                                                                                                                       \
    SingletonContainer::QueryName ## Query *copiedQueryPointer;                                                        \
    {                                                                                                                  \
        SingletonContainer::QueryName ## Query initialQuery = originalReference->QueryName ();                         \
        originalReference = nullptr;                                                                                   \
        CHECK (IsContainerInitialized (deck));                                                                         \
                                                                                                                       \
        SingletonContainer::QueryName ## Query movedQuery {std::move (initialQuery)};                                  \
        CHECK (IsContainerInitialized (deck));                                                                         \
        copiedQueryPointer = new SingletonContainer::QueryName ## Query (movedQuery);                                  \
    }                                                                                                                  \
                                                                                                                       \
    CHECK (IsContainerInitialized (deck));                                                                             \
    delete copiedQueryPointer;                                                                                         \
    CHECK (!IsContainerInitialized (deck));                                                                            \
}

QUERY_REFERENCE_MANIPULATION_TEST (Fetch)

QUERY_REFERENCE_MANIPULATION_TEST (Modify)

END_SUITE