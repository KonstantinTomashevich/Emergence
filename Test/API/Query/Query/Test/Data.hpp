#pragma once

#include <vector>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/Scenario.hpp>

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

inline const BoundingBox BOX_MIN_10_8_4_MAX_11_9_5
    {
        10.0f, 8.0f, 4.0f,
        11.0f, 9.0f, 5.0f
    };

inline const BoundingBox BOX_MIN_M2_1_0_MAX_0_4_2
    {
        -2.0f, 1.0f, 0.0f,
        0.0f, 4.0f, 2.0f
    };

inline const BoundingBox BOX_MIN_15_8_50_MAX_19_11_60
    {
        15.0f, 8.0f, 50.0f,
        19.0f, 11.0f, 60.0f
    };

inline const ScreenRect RECT_MIN_200_300_MAX_300_400
    {
        200, 300,
        300, 400
    };

inline const ScreenRect RECT_MIN_0_0_MAX_500_200
    {
        0, 0,
        500, 200
    };

inline const ScreenRect RECT_MIN_700_400_MAX_1000_600
    {
        700, 400,
        1000, 600
    };

std::vector <Sources::Volumetric::Dimension> GetDimensions2D ();

std::vector <Sources::Volumetric::Dimension> GetDimensions3D ();

std::vector <Sources::Volumetric::Dimension> GetDimensionsScreenRect ();

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