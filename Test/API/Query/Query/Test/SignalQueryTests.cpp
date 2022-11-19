#include <Assert/Assert.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/SignalQueryTests.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Query::Test::SignalQuery
{
using namespace Tasks;

constexpr uint8_t FLAG_PLAYER_ID_SOURCE = 1u << 0u;

constexpr uint32_t PLAYER_ID_SIGNALED_VALUE = 2u;

constexpr uint8_t FLAG_PLAYER_IMMOBILIZED_SOURCE = 1u << 1u;

constexpr uint8_t PLAYER_IMMOBILIZED_SIGNALED_VALUE = Player::Status::FLAG_IMMOBILIZED;

static Storage RequestStorage (const Container::Vector<const void *> &_objects, uint8_t _sources)
{
    Storage storage {Player::Reflect ().mapping, _objects, {}};

    EMERGENCE_ASSERT (_sources > 0u);
    if (_sources & FLAG_PLAYER_ID_SOURCE)
    {
        storage.sources.emplace_back (Sources::Signal {
            "playerId", Player::Reflect ().id, array_cast<uint32_t, sizeof (uint64_t)> (PLAYER_ID_SIGNALED_VALUE)});
    }

    if (_sources & FLAG_PLAYER_IMMOBILIZED_SOURCE)
    {
        storage.sources.emplace_back (
            Sources::Signal {"playerImmobilized", Player::Reflect ().immobilized,
                             array_cast<uint8_t, sizeof (uint64_t)> (PLAYER_IMMOBILIZED_SIGNALED_VALUE)});
    }

    return storage;
}

Scenario SimpleLookup ()
{
    return {{
                RequestStorage (
                    {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QuerySignalToRead {{"playerId", "0"}},
                CursorCheck {"0", &XAVIER_2_ARCHER_ALIVE_POISONED},
                CursorIncrement {"0"},
                CursorCheck {"0", nullptr},
            }};
}

Scenario NoSignaled ()
{
    return {{
                RequestStorage (
                    {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_0_MAGE_ALIVE_IMMOBILIZED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QuerySignalToRead {{"playerId", "0"}},
                CursorCheck {"0", nullptr},
            }};
}

Scenario MultipleSignaled ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED,
                                 &KARL_2_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED},
                                FLAG_PLAYER_ID_SOURCE),
            },
            {
                QuerySignalToRead {{"playerId", "0"}},
                CursorCheckAllUnordered {"0", {&KARL_2_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED}},
            }};
}

Scenario BitSignal ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &HUGO_0_KNIGHT_IMMOBILIZED,
                                 &KARL_2_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED},
                                FLAG_PLAYER_IMMOBILIZED_SOURCE),
            },
            {
                QuerySignalToRead {{"playerImmobilized", "0"}},
                CursorCheckAllUnordered {"0", {&HUGO_0_KNIGHT_IMMOBILIZED, &KARL_2_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario EditSignaled ()
{
    return {{
                RequestStorage (
                    {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QuerySignalToEdit {{"playerId", "0"}},
                CursorCheck {"0", &XAVIER_2_ARCHER_ALIVE_POISONED},
                CursorEdit {"0", &HUGO_0_KNIGHT_IMMOBILIZED},
                CursorClose {"0"},
                QuerySignalToRead {{"playerId", "0"}},
                CursorCheck {"0", nullptr},
            }};
}

Scenario DeleteSignaled ()
{
    return {{
                RequestStorage (
                    {&HUGO_0_KNIGHT_ALIVE_STUNNED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &XAVIER_2_ARCHER_ALIVE_POISONED},
                    FLAG_PLAYER_ID_SOURCE),
            },
            {
                QuerySignalToEdit {{"playerId", "0"}},
                CursorCheck {"0", &XAVIER_2_ARCHER_ALIVE_POISONED},
                CursorDeleteObject {"0"},
                CursorCheck {"0", nullptr},
                CursorClose {"0"},
                QuerySignalToRead {{"playerId", "0"}},
                CursorCheck {"0", nullptr},
            }};
}

Scenario MultipleSourcesEdition ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &HUGO_0_KNIGHT_IMMOBILIZED,
                                 &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_2_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_IMMOBILIZED_SOURCE),
            },
            {
                QuerySignalToRead {{"playerImmobilized", "1"}},
                CursorCheckAllUnordered {
                    "1", {&HUGO_0_KNIGHT_IMMOBILIZED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_2_MAGE_ALIVE_IMMOBILIZED}},
                CursorClose {"1"},

                QuerySignalToEdit {{"playerId", "0"}},
                CursorCheck {"0", &KARL_2_MAGE_ALIVE_IMMOBILIZED},
                CursorEdit {"0", &XAVIER_2_ARCHER_ALIVE_POISONED},
                CursorClose {"0"},

                QuerySignalToRead {{"playerId", "0"}},
                CursorCheckAllUnordered {"0", {&XAVIER_2_ARCHER_ALIVE_POISONED}},
                QuerySignalToRead {{"playerImmobilized", "1"}},
                CursorCheckAllUnordered {"1", {&HUGO_0_KNIGHT_IMMOBILIZED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            }};
}

Scenario MultipleSourcesDeletion ()
{
    return {{
                RequestStorage ({&HUGO_0_KNIGHT_ALIVE_STUNNED, &HUGO_0_KNIGHT_IMMOBILIZED,
                                 &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_2_MAGE_ALIVE_IMMOBILIZED},
                                FLAG_PLAYER_ID_SOURCE | FLAG_PLAYER_IMMOBILIZED_SOURCE),
            },
            {
                QuerySignalToRead {{"playerImmobilized", "1"}},
                CursorCheckAllUnordered {
                    "1", {&HUGO_0_KNIGHT_IMMOBILIZED, &KARL_1_MAGE_ALIVE_IMMOBILIZED, &KARL_2_MAGE_ALIVE_IMMOBILIZED}},
                CursorClose {"1"},

                QuerySignalToEdit {{"playerId", "0"}},
                CursorCheck {"0", &KARL_2_MAGE_ALIVE_IMMOBILIZED},
                CursorDeleteObject {"0"},
                CursorCheck {"0", nullptr},
                CursorClose {"0"},

                QuerySignalToRead {{"playerId", "0"}},
                CursorCheckAllUnordered {"0", {}},
                QuerySignalToRead {{"playerImmobilized", "1"}},
                CursorCheckAllUnordered {"1", {&HUGO_0_KNIGHT_IMMOBILIZED, &KARL_1_MAGE_ALIVE_IMMOBILIZED}},
            }};
}
} // namespace Emergence::Query::Test::SignalQuery
