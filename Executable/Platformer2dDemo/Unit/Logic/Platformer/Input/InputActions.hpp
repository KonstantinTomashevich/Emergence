#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/Input/InputAction.hpp>

struct Platformer2dDemoLogicApi PlatformerInputGroups final
{
    static const Emergence::Memory::UniqueString MOVEMENT_INPUT;
};

struct Platformer2dDemoLogicApi PlatformerInputActions final
{
    static const Emergence::Celerity::InputAction DIRECTED_MOVEMENT;

    static const Emergence::Celerity::InputAction MOVEMENT_ROLL;
};
