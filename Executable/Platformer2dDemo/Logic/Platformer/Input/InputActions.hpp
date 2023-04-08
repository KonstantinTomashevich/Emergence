#pragma once

#include <Celerity/Input/InputAction.hpp>

struct PlatformerInputGroups final
{
    static const Emergence::Memory::UniqueString MOVEMENT_INPUT;
};

struct PlatformerInputActions final
{
    static const Emergence::Celerity::InputAction DIRECTED_MOVEMENT;

    static const Emergence::Celerity::InputAction MOVEMENT_ROLL;
};
