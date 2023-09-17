#include <Assert/Assert.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/ValueQueryTests.hpp>

namespace Emergence::Query::Test::ValueQuery
{
using namespace Tasks;

constexpr std::uint8_t FLAG_PLAYER_ID_SOURCE = 1u;

constexpr std::uint8_t FLAG_PLAYER_NAME_SOURCE = 1u << 1u;

constexpr std::uint8_t FLAG_PLAYER_CLASS_ID_SOURCE = 1u << 2u;

constexpr std::uint8_t FLAG_PLAYER_NAME_AND_ID_SOURCE = 1u << 3u;

constexpr std::uint8_t FLAG_PLAYER_ALIVE_SOURCE = 1u << 4u;

constexpr std::uint8_t FLAG_PLAYER_ALIVE_AND_STUNNED_SOURCE = 1u << 5u;

static Storage RequestStorage (const Container::Vector<const void *> &_objects, std::uint8_t _sources)
{
    Storage storage {Player::Reflect ().mapping, _objects, {}};

    EMERGENCE_ASSERT (_sources > 0u);
    if (_sources & FLAG_PLAYER_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerId", {Player::Reflect ().id}});
    }

    if (_sources & FLAG_PLAYER_NAME_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerName", {Player::Reflect ().name}});
    }

    if (_sources & FLAG_PLAYER_CLASS_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerClassId", {Player::Reflect ().classId}});
    }

    if (_sources & FLAG_PLAYER_NAME_AND_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerNameAndId",
                                                      {
                                                          Player::Reflect ().name,
                                                          Player::Reflect ().id,
                                                      }});
    }

    if (_sources & FLAG_PLAYER_ALIVE_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerAlive", {Player::Reflect ().alive}});
    }

    if (_sources & FLAG_PLAYER_ALIVE_AND_STUNNED_SOURCE)
    {
        storage.sources.emplace_back (Sources::Value {"playerAliveAndStunned",
                                                      {
                                                          Player::Reflect ().alive,
                                                          Player::Reflect ().stunned,
                                                      }});
    }

    return storage;
}

Scenario SimpleLookup ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}, FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheck {"0", &HUGO_0_KNIGHT_ALIVE_STUNNED},
                CursorIncrement {"0"},
                CursorCheck {"0", nullptr},
            }};
}

Scenario LookupForNonExistentObject ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}, FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},
                QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheckAllUnordered {"1", {&KARL_1_MAGE_ALIVE_IMMOBILIZED}},
                QueryValueToRead {{{"playerId", "2"}, &Queries::ID_2}},
                CursorCheck {"2", nullptr},
            }};
}

Scenario LookupForMany ()
{
    return {
        {
            RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &HUGO_0_KNIGHT_IMMOBILIZED},
                            FLAG_PLAYER_ID_SOURCE),
        },
        {
            QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
            CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_ALIVE_STUNNED, &HUGO_0_KNIGHT_IMMOBILIZED}},
        }};
}

Scenario LookupAndEdit ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}, FLAG_PLAYER_ID_SOURCE),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_MAGE_ALIVE_IMMOBILIZED},
                CursorIncrement {"1"},
                CursorCheck {"1", nullptr},
                CursorClose {"1"},
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_0_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario OnStringField ()
{
    return {
        {
            RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}, FLAG_PLAYER_NAME_SOURCE),
        },
        {
            QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
            CursorCheckAllUnordered {"karl", {&KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
            CursorCheckAllUnordered {"hugo", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},
        }};
}

Scenario OnUniqueStringField ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_CLASS_ID_SOURCE),
            },
            {
                QueryValueToRead {{{"playerClassId", "Mage"}, &Queries::MAGE}},
                CursorCheckAllUnordered {"Mage", {&KARL_1_MAGE_ALIVE_IMMOBILIZED}},
                QueryValueToRead {{{"playerClassId", "Knight"}, &Queries::KNIGHT}},
                CursorCheckAllUnordered {"Knight", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},
                QueryValueToRead {{{"playerClassId", "Archer"}, &Queries::ARCHER}},
                CursorCheck {"Archer", nullptr},
            }};
}

Scenario OnTwoFields ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_NAME_AND_ID_SOURCE),
            },
            {
                QueryValueToRead {{{"playerNameAndId", "hugo1"}, &Queries::HUGO_1}},
                CursorCheck {"hugo1", nullptr},
                QueryValueToRead {{{"playerNameAndId", "karl1"}, &Queries::KARL_1}},
                CursorCheckAllUnordered {"karl1", {&KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario OnBitField ()
{
    return {
        {
            RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &HUGO_0_KNIGHT_IMMOBILIZED},
                            FLAG_PLAYER_ALIVE_SOURCE),
        },
        {
            QueryValueToRead {{{"playerAlive", "alive"}, &Queries::ALIVE}},
            CursorCheckAllUnordered {"alive", {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            CursorClose {"alive"},

            QueryValueToEdit {{{"playerAlive", "dead"}, &Queries::DEAD}},
            CursorCheck {"dead", &HUGO_0_KNIGHT_IMMOBILIZED},
            CursorEdit {"dead", &KARL_0_MAGE_ALIVE_IMMOBILIZED},
            CursorIncrement {"dead"},
            CursorCheck {"dead", nullptr},
            CursorClose {"dead"},

            QueryValueToRead {{{"playerAlive", "alive"}, &Queries::ALIVE}},
            CursorCheckAllUnordered {
                "alive",
                {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_0_MAGE_ALIVE_IMMOBILIZED}},

            QueryValueToRead {{{"playerAlive", "dead"}, &Queries::DEAD}},
            CursorCheck {"dead", nullptr},
        }};
}

Scenario OnTwoBitFields ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_ALIVE_AND_STUNNED_SOURCE),
            },
            {
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndStunned"}, &Queries::ALIVE_AND_STUNNED}},
                CursorCheckAllUnordered {"aliveAndStunned", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndNotStunned"}, &Queries::ALIVE_AND_NOT_STUNNED}},
                CursorCheckAllUnordered {"aliveAndNotStunned", {&KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario MultipleSourcesEdition ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_NAME_SOURCE | FLAG_PLAYER_NAME_AND_ID_SOURCE),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_MAGE_ALIVE_IMMOBILIZED},
                CursorClose {"1"},

                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_0_MAGE_ALIVE_IMMOBILIZED}},

                QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
                CursorCheckAllUnordered {"karl", {&KARL_0_MAGE_ALIVE_IMMOBILIZED}},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
                CursorCheckAllUnordered {"hugo", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},

                QueryValueToRead {{{"playerNameAndId", "karl1"}, &Queries::KARL_1}},
                CursorCheck {"karl1", nullptr},

                QueryValueToRead {{{"playerNameAndId", "karl0"}, &Queries::KARL_0}},
                CursorCheckAllUnordered {"karl0", {&KARL_0_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario MultipleSourcesDeletion ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_NAME_SOURCE),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_MAGE_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_MAGE_ALIVE_IMMOBILIZED},
                CursorDeleteObject {"1"},
                CursorClose {"1"},

                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},

                QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
                CursorCheck {"karl", nullptr},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
                CursorCheckAllUnordered {"hugo", {&HUGO_0_KNIGHT_ALIVE_STUNNED}},
            }};
}
} // namespace Emergence::Query::Test::ValueQuery
