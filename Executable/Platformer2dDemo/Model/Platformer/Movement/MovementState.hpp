#pragma once

enum class MovementState : uint8_t
{
    IDLE = 0,
    RUN,
    CROUCH,
    JUMP,
    FALL,
    ROLL,
    SLIDE,
    BLOCKED,
};

enum class MovementStateFlag : uint8_t
{
    NONE = 0u,
    IDLE = 1u << 0u,
    RUN = 1u << 1u,
    CROUCH = 1u << 2u,
    JUMP = 1u << 3u,
    FALL = 1u << 4u,
    ROLL = 1u << 5u,
    SLIDE = 1u << 6u,
    BLOCKED = 1u << 7u,
};
