#pragma once

#include <Celerity/Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores input action and its dispatch setting until action is dispatched to subscribers.
/// \details Stored in short term container because holders lifetime usually ends when action is dispatched,
///          unless it is an InputActionDispatchType::FIXED_PERSISTENT. Fixed persistent holders are removed
///          during next normal frame after being dispatched.
struct InputActionHolder final
{
    /// \brief Action to be dispatched.
    InputAction action;

    /// \brief Describes how action should be dispatched.
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;

    union
    {
        /// \brief Internal value for InputActionDispatchType::FIXED_PERSISTENT logic.
        bool fixedDispatchDone = false;
    };

    bool operator== (const InputActionHolder &_other) const;

    bool operator!= (const InputActionHolder &_other) const;

    struct Reflection final
    {
        StandardLayout::FieldId action;
        StandardLayout::FieldId dispatchType;
        StandardLayout::FieldId fixedDispatchDone;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
