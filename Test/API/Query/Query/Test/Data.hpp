#pragma once

#include <Query/Test/DataTypes.hpp>

namespace Emergence::Query::Test
{
inline const Player HUGO_0_ALIVE_STUNNED
    {
        0u,
        {"hugo"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_STUNNED
    };

inline const Player DUPLICATE_0_IMMOBILIZED
    {
        0u,
        {"hugo"},
        Player::Status::FLAG_IMMOBILIZED
    };

inline const Player KARL_1_ALIVE_IMMOBILIZED
    {
        1u,
        {"karl"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_IMMOBILIZED
    };

inline const Player KARL_0_ALIVE_IMMOBILIZED
    {
        0u,
        {"karl"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_IMMOBILIZED
    };

inline const Player XAVIER_2_ALIVE_POISONED
    {
        2u,
        {"xavier"},
        Player::Status::FLAG_ALIVE | Player::Status::FLAG_POISONED
    };

inline const AllFieldTypesStructure ALL_FIELD_TYPES_0
    {
        -3,
        258,
        -53400,
        6,

        1u,
        13u,
        79u,
        1111u,

        89.0f,
        12.3458,

        {13u, 12u, 15u, 21u},
        {"hugo"},
    };

inline const AllFieldTypesStructure ALL_FIELD_TYPES_1
    {
        1,
        -233,
        170,
        182634,

        13u,
        1563u,
        173656u,
        1234u,

        111.0f,
        6.2356,

        {67u, 12u, 15u, 21u},
        {"karl"},
    };

inline const AllFieldTypesStructure ALL_FIELD_TYPES_2
    {
        0,
        2,
        -5,
        63674896,

        4u,
        37u,
        12341u,
        17939471u,

        45.0f,
        17.3458,

        {13u, 15u, 15u, 21u},
        {"xavier"},
    };

namespace Queries
{
inline const PlayerId ID_0 {0u};

inline const PlayerId ID_1 {1u};

inline const PlayerId ID_2 {2u};

inline const PlayerId ID_3 {3u};

inline const PlayerName HUGO {{"hugo"}};

inline const PlayerName IORAN {{"ioran"}};

inline const PlayerName KARL {{"karl"}};

inline const PlayerName XAVIER {{"xavier"}};

inline const PlayerName ZANEK {{"zanek"}};

inline const PlayerNameAndId HUGO_1 {{"hugo"}, 1u};

inline const PlayerNameAndId KARL_0 {{"karl"}, 0u};

inline const PlayerNameAndId KARL_1 {{"karl"}, 1u};

inline const PlayerAlive ALIVE {Player::Status::FLAG_ALIVE};

inline const PlayerAlive DEAD {0u};

inline const PlayerAliveAndStunned ALIVE_AND_STUNNED {Player::Status::FLAG_ALIVE, Player::Status::FLAG_STUNNED};

inline const PlayerAliveAndStunned ALIVE_AND_NOT_STUNNED {Player::Status::FLAG_ALIVE, 0u};
} // namespace Queries
} // namespace Emergence::Query::Test