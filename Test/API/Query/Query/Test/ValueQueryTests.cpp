#include <Query/Test/Data.hpp>
#include <Query/Test/Common.hpp>
#include <Query/Test/ValueQueryTests.hpp>

namespace Emergence::Query::Test::ValueQuery
{
using namespace Tasks;

Sources::Value PlayerIdSourceWithObjects (const std::vector <const void *> &_objects)
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

Sources::Value PlayerNameSourceWithObjects (const std::vector <const void *> &_objects)
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

Sources::Value PlayerNameAndIdSourceWithObjects (const std::vector <const void *> &_objects)
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
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                CursorCheck {"0", &hugo_0_alive_stunned},
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
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            TestCursorCopyAndMove (
                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                QueryValueToEdit {{{"playerId", "0"}, &Queries::id0}},
                &hugo_0_alive_stunned, nullptr, &hugo_0_alive_stunned)
        };
}

Scenario LookupForNonExistentRecord () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                CursorCheckAllUnordered {"0", {&hugo_0_alive_stunned}},
                QueryValueToRead {{{"playerId", "1"}, &Queries::id1}},
                CursorCheckAllUnordered {"1", {&karl_1_alive_immobilized}},
                QueryValueToRead {{{"playerId", "2"}, &Queries::id2}},
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
                    {&hugo_0_alive_stunned, &karl_1_alive_immobilized, &duplicate_0_immobilized}),
            },
            {
                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                CursorCheckAllUnordered {"0", {&hugo_0_alive_stunned, &duplicate_0_immobilized}},
            }
        };
}

Scenario LookupAndEdit () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::id1}},
                CursorCheck {"1", &karl_1_alive_immobilized},
                CursorEdit {"1", &karl_0_alive_immobilized},
                CursorIncrement {"1"},
                CursorCheck {"1", nullptr},
                CursorClose {"1"},
                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                CursorCheckAllUnordered {"0", {&hugo_0_alive_stunned, &karl_0_alive_immobilized}},
            }
        };
}

Scenario OnStringField () noexcept
{
    return
        {
            {
                PlayerNameSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToRead {{{"playerName", "karl"}, &Queries::karl}},
                CursorCheckAllUnordered {"karl", {&karl_1_alive_immobilized}},
                QueryValueToRead {{{"playerName", "hugo"}, &Queries::hugo}},
                CursorCheckAllUnordered {"hugo", {&hugo_0_alive_stunned}},
            }
        };
}

Scenario OnTwoFields () noexcept
{
    return
        {
            {
                PlayerNameAndIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToRead {{{"playerNameAndId", "hugoEntity1"}, &Queries::hugo_1}},
                CursorCheck {"hugoEntity1", nullptr},
                QueryValueToRead {{{"playerNameAndId", "karlEntity1"}, &Queries::karl_1}},
                CursorCheckAllUnordered {"karlEntity1", {&karl_1_alive_immobilized}},
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
                                &hugo_0_alive_stunned,
                                &karl_1_alive_immobilized,
                                &duplicate_0_immobilized,
                            }
                        },
                        {
                            Player::Reflection::alive,
                        }
                    }
            },
            {
                QueryValueToRead {{{"playerAlive", "alive"}, &Queries::alive}},
                CursorCheckAllUnordered {"alive", {&hugo_0_alive_stunned, &karl_1_alive_immobilized}},
                CursorClose {"alive"},

                QueryValueToEdit {{{"playerAlive", "dead"}, &Queries::dead}},
                CursorCheck {"dead", &duplicate_0_immobilized},
                CursorEdit {"dead", &karl_0_alive_immobilized},
                CursorIncrement {"dead"},
                CursorCheck {"dead", nullptr},
                CursorClose {"dead"},

                QueryValueToRead {{{"playerAlive", "alive"}, &Queries::alive}},
                CursorCheckAllUnordered {
                    "alive", {&hugo_0_alive_stunned, &karl_1_alive_immobilized, &karl_0_alive_immobilized}},

                QueryValueToRead {{{"playerAlive", "dead"}, &Queries::dead}},
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
                                &hugo_0_alive_stunned,
                                &karl_1_alive_immobilized,
                            }
                        },
                        {
                            Player::Reflection::alive,
                            Player::Reflection::stunned,
                        }
                    }
            },
            {
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndStunned"}, &Queries::aliveAndStunned}},
                CursorCheckAllUnordered {"aliveAndStunned", {&hugo_0_alive_stunned}},
                QueryValueToRead {{{"playerAliveAndStunned", "aliveAndNotStunned"}, &Queries::aliveAndNotStunned}},
                CursorCheckAllUnordered {"aliveAndNotStunned", {&karl_1_alive_immobilized}},
            }
        };
}

Scenario MultipleIndicesEdition () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
                PlayerNameSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
                PlayerNameAndIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToEdit {{{"entityId", "1"}, &Queries::id1}},
                CursorCheck {"1", &karl_1_alive_immobilized},
                CursorEdit {"1", &karl_0_alive_immobilized},
                CursorClose {"1"},

                QueryValueToRead {{{"entityId", "0"}, &Queries::id0}},
                CursorCheckAllUnordered {"0", {&hugo_0_alive_stunned, &karl_0_alive_immobilized}},

                QueryValueToRead {{{"entityId", "1"}, &Queries::id1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::karl}},
                CursorCheckAllUnordered {"karl", {&karl_0_alive_immobilized}},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::hugo}},
                CursorCheckAllUnordered {"hugo", {&hugo_0_alive_stunned}},

                QueryValueToRead {{{"playerNameAndId", "karlEntity1"}, &Queries::karl_1}},
                CursorCheck {"karlEntity1", nullptr},

                QueryValueToRead {{{"playerNameAndId", "karlEntity0"}, &Queries::karl_0}},
                CursorCheckAllUnordered {"karlEntity0", {&karl_0_alive_immobilized}},
            }
        };
}

Scenario MultipleIndicesDeletion () noexcept
{
    return
        {
            {
                PlayerIdSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
                PlayerNameSourceWithObjects ({&hugo_0_alive_stunned, &karl_1_alive_immobilized}),
            },
            {
                QueryValueToEdit {{{"playerId", "1"}, &Queries::id1}},
                CursorCheck {"1", &karl_1_alive_immobilized},
                CursorEdit {"1", &karl_0_alive_immobilized},
                CursorDeleteRecord {"1"},
                CursorClose {"1"},

                QueryValueToRead {{{"playerId", "0"}, &Queries::id0}},
                CursorCheckAllUnordered {"0", {&hugo_0_alive_stunned}},

                QueryValueToRead {{{"playerId", "1"}, &Queries::id1}},
                CursorCheck {"1", nullptr},

                QueryValueToRead {{{"playerName", "karl"}, &Queries::karl}},
                CursorCheck {"karl", nullptr},

                QueryValueToRead {{{"playerName", "hugo"}, &Queries::hugo}},
                CursorCheckAllUnordered {"hugo", {&hugo_0_alive_stunned}},
            }
        };
}
} // namespace Emergence::Query::Test::ValueQuery