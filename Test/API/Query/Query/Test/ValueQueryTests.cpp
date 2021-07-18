#include <Query/Test/Common.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/ValueQueryTests.hpp>

namespace Emergence::Query::Test::ValueQuery
{
using namespace Tasks;

static Sources::Value PlayerIdSourceWithObjects (const std::vector <const void *> &_objects)
{
    return Sources::Value
        {
            {
                "playerId",
                Player::Reflection::GetMapping (),
                _objects
            },
            {
                Player::Reflection::id,
            }
        };
}

static Sources::Value PlayerNameSourceWithObjects (const std::vector <const void *> &_objects)
{
    return Sources::Value
        {
            {
                "playerName",
                Player::Reflection::GetMapping (),
                _objects
            },
            {
                Player::Reflection::name,
            }
        };
}

static Sources::Value PlayerNameAndIdSourceWithObjects (const std::vector <const void *> &_objects)
{
    return Sources::Value
        {
            {
                "playerNameAndId",
                Player::Reflection::GetMapping (),
                _objects
            },
            {
                Player::Reflection::name,
                Player::Reflection::id,
            }
        };
}

Scenario SimpleLookup () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheck {"0", &HUGO_0_ALIVE_STUNNED},
                CursorIncrement {"0"},
                CursorCheck {"0", nullptr},
            }
        };
}

Scenario CursorManipulations () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            TestCursorCopyAndMove (
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                QueryValueToEdit {{{"playerId", "0"}, &Queries::ID_0}},
                &HUGO_0_ALIVE_STUNNED, nullptr, &HUGO_0_ALIVE_STUNNED)
        };
}

Scenario LookupForNonExistentRecord () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_ALIVE_STUNNED}},
                QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheckAllUnordered {"1", {&KARL_1_ALIVE_IMMOBILIZED}},
                QueryValueToRead {{{"playerId", "2"}, &Queries::ID_2}},
                CursorCheck {"2", nullptr},
            }
        };
}

Scenario LookupForMany () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects (
                    {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &DUPLICATE_0_IMMOBILIZED}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_ALIVE_STUNNED, &DUPLICATE_0_IMMOBILIZED}},
            }
        };
}

Scenario LookupAndEdit () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_ALIVE_IMMOBILIZED},
                CursorIncrement {"1"},
                CursorCheck {"1", nullptr},
                CursorClose {"1"},
                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_ALIVE_STUNNED, &KARL_0_ALIVE_IMMOBILIZED}},
            }
        };
}

Scenario OnStringField () noexcept
{
    return
        {
            {
                PlayerNameSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
                CursorCheckAllUnordered {"karl", {&KARL_1_ALIVE_IMMOBILIZED}},
                QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
                CursorCheckAllUnordered {"hugo", {&HUGO_0_ALIVE_STUNNED}},
            }
        };
}

Scenario OnTwoFields () noexcept
{
    return
        {
            {
                PlayerNameAndIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToRead {{{"playerNameAndId", "hugoEntity1"}, &Queries::HUGO_1}},
                CursorCheck {"hugoEntity1", nullptr},
                QueryValueToRead {{{"playerNameAndId", "karlEntity1"}, &Queries::KARL_1}},
                CursorCheckAllUnordered {"karlEntity1", {&KARL_1_ALIVE_IMMOBILIZED}},
            }
        };
}

Scenario OnBitField () noexcept
{
    return
        {
            {
                Sources::Value
                    {
                        {
                            "playerAlive",
                            Player::Reflection::GetMapping (),
                            {
                                &HUGO_0_ALIVE_STUNNED,
                                &KARL_1_ALIVE_IMMOBILIZED,
                                &DUPLICATE_0_IMMOBILIZED,
                            }
                        },
                        {
                            Player::Reflection::alive,
                        }
                    }
            },
            {
                QueryValueToRead {{{"playerAlive", "alive"}, &Queries::ALIVE}},
                CursorCheckAllUnordered {"alive", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}},
                CursorClose {"alive"},

                QueryValueToEdit {{{"playerAlive", "dead"}, &Queries::DEAD}},
                CursorCheck {"dead", &DUPLICATE_0_IMMOBILIZED},
                CursorEdit {"dead", &KARL_0_ALIVE_IMMOBILIZED},
                CursorIncrement {"dead"},
                CursorCheck {"dead", nullptr},
                CursorClose {"dead"},

                QueryValueToRead {{{"playerAlive", "alive"}, &Queries::ALIVE}},
                CursorCheckAllUnordered {
                    "alive", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED, &KARL_0_ALIVE_IMMOBILIZED}},

                QueryValueToRead {{{"playerAlive", "dead"}, &Queries::DEAD}},
                CursorCheck {"dead", nullptr},
            }
        };
}

Scenario OnTwoBitFields () noexcept
{
    return
        {
            {
                Sources::Value
                    {
                        {
                            "playerAliveAndStunned",
                            Player::Reflection::GetMapping (),
                            {
                                &HUGO_0_ALIVE_STUNNED,
                                &KARL_1_ALIVE_IMMOBILIZED,
                            }
                        },
                        {
                            Player::Reflection::alive,
                            Player::Reflection::stunned,
                        }
                    }
            },
            {
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndStunned"}, &Queries::ALIVE_AND_STUNNED}},
                CursorCheckAllUnordered {"aliveAndStunned", {&HUGO_0_ALIVE_STUNNED}},
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndNotStunned"}, &Queries::ALIVE_AND_NOT_STUNNED}},
                CursorCheckAllUnordered {"aliveAndNotStunned", {&KARL_1_ALIVE_IMMOBILIZED}},
            }
        };
}

Scenario MultipleIndicesEdition () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
                PlayerNameSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
                PlayerNameAndIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToEdit {{{"entityId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_ALIVE_IMMOBILIZED},
                CursorClose {"1"},

                QueryValueToRead {{{"entityId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_ALIVE_STUNNED, &KARL_0_ALIVE_IMMOBILIZED}},

                QueryValueToRead {{{"entityId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
                CursorCheckAllUnordered {"karl", {&KARL_0_ALIVE_IMMOBILIZED}},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
                CursorCheckAllUnordered {"hugo", {&HUGO_0_ALIVE_STUNNED}},

                QueryValueToRead {{{"playerNameAndId", "karlEntity1"}, &Queries::KARL_1}},
                CursorCheck {"karlEntity1", nullptr},

                QueryValueToRead {{{"playerNameAndId", "karlEntity0"}, &Queries::KARL_0}},
                CursorCheckAllUnordered {"karlEntity0", {&KARL_0_ALIVE_IMMOBILIZED}},
            }
        };
}

Scenario MultipleIndicesDeletion () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
                PlayerNameSourceWithObjects ({&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", &KARL_1_ALIVE_IMMOBILIZED},
                CursorEdit {"1", &KARL_0_ALIVE_IMMOBILIZED},
                CursorDeleteRecord {"1"},
                CursorClose {"1"},

                QueryValueToRead {{{"playerId", "0"}, &Queries::ID_0}},
                CursorCheckAllUnordered {"0", {&HUGO_0_ALIVE_STUNNED}},

                QueryValueToRead {{{"playerId", "1"}, &Queries::ID_1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::KARL}},
                CursorCheck {"karl", nullptr},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::HUGO}},
                CursorCheckAllUnordered {"hugo", {&HUGO_0_ALIVE_STUNNED}},
            }
        };
}
} // namespace Emergence::Query::Test::ValueQuery