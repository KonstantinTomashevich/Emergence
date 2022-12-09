#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Used to deliver InputAction to object that subscribed to actions using InputSubscriptionComponent.
struct InputActionComponent final
{
    /// \brief Id of an object that receives the event.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Event that is passed to subscriber object.
    InputAction action;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId action;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
