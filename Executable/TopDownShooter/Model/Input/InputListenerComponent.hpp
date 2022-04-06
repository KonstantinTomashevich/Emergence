#pragma once

#include <array>

#include <Celerity/Standard/ObjectId.hpp>

#include <Container/InplaceVector.hpp>

#include <Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

struct InputListenerComponent final
{
    constexpr static const std::size_t MAX_ACTIONS_PER_CYCLE = 16u;

    Emergence::Celerity::ObjectId objectId = Emergence::Celerity::INVALID_OBJECT_ID;
    Emergence::Container::InplaceVector<InputAction, MAX_ACTIONS_PER_CYCLE> actions;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_PER_CYCLE> actions;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};