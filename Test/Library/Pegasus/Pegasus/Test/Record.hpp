#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Pegasus::Test
{
struct AvatarInfo final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId teamId;
        static StandardLayout::FieldId classId;
    };

    uint8_t teamId = 0u;
    uint8_t classId = 0u;
};

struct AvatarRuntime
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId ammoLeft;
        static StandardLayout::FieldId health;
    };

    uint16_t ammoLeft = 0u;
    float health = 0u;
};

struct Transform final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId x;
        static StandardLayout::FieldId y;
        static StandardLayout::FieldId rotationDeg;
    };

    float x = 0.0f;
    float y = 0.0f;
    float rotationDeg = 0.0f;
};

struct Record final
{
    struct Reflection final
    {
        Reflection () = delete;

        static StandardLayout::Mapping GetMapping ();

        static StandardLayout::FieldId entityId;
        static StandardLayout::FieldId nickname;
        static StandardLayout::FieldId info;
        static StandardLayout::FieldId runtime;
        static StandardLayout::FieldId transform;
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

    static constexpr std::size_t NICKNAME_MAX_SIZE = 32u;

    uint32_t entityId = 0u;
    std::array <char, NICKNAME_MAX_SIZE> nickname = {0u};

    AvatarInfo info;
    AvatarRuntime runtime;
    Transform transform;
    uint8_t status = 0u;
};
} // namespace Emergence::Pegasus::Test
