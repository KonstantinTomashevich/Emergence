#pragma once

#include <StandardLayout/Field.hpp>
#include <StandardLayout/Mapping.hpp>

namespace Emergence::Query::Test
{
/// \brief Data structure for value and range query tests.
struct Player final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId id;
        static StandardLayout::FieldId name;

        static StandardLayout::FieldId alive;
        static StandardLayout::FieldId stunned;
        static StandardLayout::FieldId poisoned;
        static StandardLayout::FieldId immobilized;
    };

    struct Status final
    {
        Status () = delete;

        static constexpr uint8_t FLAG_ALIVE_OFFSET = 0u;
        static constexpr uint8_t FLAG_ALIVE = 1u << FLAG_ALIVE_OFFSET;

        static constexpr uint8_t FLAG_STUNNED_OFFSET = 1u;
        static constexpr uint8_t FLAG_STUNNED = 1u << FLAG_STUNNED_OFFSET;

        static constexpr uint8_t FLAG_POISONED_OFFSET = 2u;
        static constexpr uint8_t FLAG_POISONED = 1u << FLAG_POISONED_OFFSET;

        static constexpr uint8_t FLAG_IMMOBILIZED_OFFSET = 3u;
        static constexpr uint8_t FLAG_IMMOBILIZED = 1u << FLAG_IMMOBILIZED_OFFSET;
    };

    static constexpr std::size_t NAME_MAX_SIZE = 32u;

    uint32_t id = 0u;
    std::array <char, NAME_MAX_SIZE> name = {0u};
    uint8_t status = 0u;
};

/// \brief Data structure for floating-point shape and ray intersection queries.
struct BoundingBox final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId minX;
        static StandardLayout::FieldId minY;
        static StandardLayout::FieldId minZ;

        static StandardLayout::FieldId maxX;
        static StandardLayout::FieldId maxY;
        static StandardLayout::FieldId maxZ;
    };

    float minX = 0.0f;
    float minY = 0.0f;
    float minZ = 0.0f;

    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;
};

/// \brief Data structure for integer shape and ray intersection queries.
struct ScreenRect final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId minX;
        static StandardLayout::FieldId minY;

        static StandardLayout::FieldId maxX;
        static StandardLayout::FieldId maxY;
    };

    int16_t minX = 0;
    int16_t minY = 0;

    int16_t maxX = 0;
    int16_t maxY = 0;
};

/// \brief Data structure for tests, that execute all types of queries on single object type.
struct MergedRecord final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId player;
        static StandardLayout::FieldId boundingBox;
        static StandardLayout::FieldId screenRect;
    };

    Player player;
    BoundingBox boundingBox;
    ScreenRect screenRect;
};

/// \brief Data structure with all supported field archetypes, except bit fields.
///        Used to cover all comparators for range queries.
struct AllFieldTypesStructure
{
    struct Reflection final
    {
        Reflection () = delete;

        static Emergence::StandardLayout::Mapping GetMapping ();

        static Emergence::StandardLayout::FieldId int8;
        static Emergence::StandardLayout::FieldId int16;
        static Emergence::StandardLayout::FieldId int32;
        static Emergence::StandardLayout::FieldId int64;

        static Emergence::StandardLayout::FieldId uint8;
        static Emergence::StandardLayout::FieldId uint16;
        static Emergence::StandardLayout::FieldId uint32;
        static Emergence::StandardLayout::FieldId uint64;

        static Emergence::StandardLayout::FieldId floating;
        static Emergence::StandardLayout::FieldId doubleFloating;

        static Emergence::StandardLayout::FieldId block;
        static Emergence::StandardLayout::FieldId string;
    };

    int8_t int8 = 0;
    int16_t int16 = 0;
    int32_t int32 = 0;
    int64_t int64 = 0;

    uint8_t uint8 = 0u;
    uint16_t uint16 = 0u;
    uint32_t uint32 = 0u;
    uint64_t uint64 = 0u;

    float floating = 0.0f;
    double doubleFloating = 0.0;

    std::array <uint8_t, 4> block {};
    std::array <char, 24> string {};
};

namespace Queries
{
struct PlayerId final
{
    decltype (Player::id) id = 0u;
};

struct PlayerName final
{
    decltype (Player::name) name {0u};
};

struct PlayerNameAndId final
{
    decltype (Player::name) name {0u};
    decltype (Player::id) id = 0u;
};

struct PlayerAlive
{
    uint8_t aliveFlag = 0u;
};

struct PlayerAliveAndStunned
{
    uint8_t aliveFlag = 0u;
    uint8_t stunnedFlag = 0u;
};
} // namespace Queries
} // namespace Emergence::Query::Test
