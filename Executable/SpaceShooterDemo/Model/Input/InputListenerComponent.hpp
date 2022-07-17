#pragma once

#include <array>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/InplaceVector.hpp>

#include <Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

struct InputListenerComponent final
{
    static constexpr const std::size_t MAX_ACTIONS_PER_CYCLE = 16u;

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Container::InplaceVector<InputAction, MAX_ACTIONS_PER_CYCLE> actions;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId actions;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
