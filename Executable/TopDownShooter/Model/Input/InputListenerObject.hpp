#pragma once

#include <array>

#include <Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

struct InputListenerObject final
{
    constexpr static const std::size_t MAX_ACTIONS_PER_CYCLE = 16u;

    std::uint64_t objectId = 0u;
    std::array<InputAction, MAX_ACTIONS_PER_CYCLE> actions;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_PER_CYCLE> actions;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
