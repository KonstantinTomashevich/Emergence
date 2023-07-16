#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Subscribes its object to all input actions of given group.
struct InputSubscriptionComponent final
{
    /// \brief Id of the object that will receive actions.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Only actions with InputAction::group value equal to this will be added to the object.
    /// \details In case of more than one subscription per object, input actions with InputAction::group
    ///          equal to at least one of the InputSubscriptionComponent::group will be added.
    Memory::UniqueString group;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId group;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
