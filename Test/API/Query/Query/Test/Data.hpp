#pragma once

#include <Query/Test/DataTypes.hpp>

namespace Emergence::Query::Test
{
inline const Player hugo_0_alive_stunned
    {
        0u,
        {"hugo"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_STUNNED
    };

inline const Player duplicate_0_immobilized
    {
        0u,
        {"hugo"},
        Player::Status::FLAG_IMMOBILIZED
    };

inline const Player karl_1_alive_immobilized
    {
        1u,
        {"karl"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_IMMOBILIZED
    };

inline const Player karl_0_alive_immobilized
    {
        0u,
        {"karl"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_IMMOBILIZED
    };

namespace Queries
{
inline const PlayerId id0 {0u};

inline const PlayerId id1 {1u};

inline const PlayerId id2 {2u};

inline const PlayerName hugo {{"hugo"}};

inline const PlayerName karl {{"karl"}};

inline const PlayerNameAndId hugo_1 {{"hugo"}, 1u};

inline const PlayerNameAndId karl_0 {{"karl"}, 0u};

inline const PlayerNameAndId karl_1 {{"karl"}, 1u};

inline const PlayerAlive alive {Player::Status::FLAG_ALIVE};

inline const PlayerAlive dead {0u};

inline const PlayerAliveAndStunned aliveAndStunned {Player::Status::FLAG_ALIVE, Player::Status::FLAG_STUNNED};

inline const PlayerAliveAndStunned aliveAndNotStunned {Player::Status::FLAG_ALIVE, 0u};
} // namespace Queries
} // namespace Emergence::Query::Test