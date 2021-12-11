#pragma once

#include <array>

#include <Input/InputAction.hpp>

#include <Shared/Constants.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

struct InputListenerObject final
{
    constexpr static const std::size_t MAX_ACTIONS_PER_CYCLE = 16u;

    std::uintptr_t objectId = INVALID_OBJECT_ID;
    Emergence::InplaceVector<InputAction, MAX_ACTIONS_PER_CYCLE> actions;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_PER_CYCLE> actions;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivially_destructible_v<InputListenerObject>);
