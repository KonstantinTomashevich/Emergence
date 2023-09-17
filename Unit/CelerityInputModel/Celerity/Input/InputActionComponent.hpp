#pragma once

#include <CelerityInputModelApi.hpp>

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Used to deliver InputAction to object that subscribed to actions using InputSubscriptionComponent.
/// \details Although this component lifetime is exactly one update, it is still placed into long term storage
///          in order to be properly indexed.
struct CelerityInputModelApi InputActionComponent final
{
    /// \brief Id of an object that receives the event.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Action that is passed to subscriber object.
    InputAction action;

    struct CelerityInputModelApi Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId action;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
