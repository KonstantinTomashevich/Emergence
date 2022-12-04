#pragma once

#include <Celerity/Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \details Stored in short term container because holders lifetime usually ends when action is dispatched,
///          unless it is a InputActionDispatchType::FIXED_PERSISTENT. Fixed persistent holders are removed
///          during next normal frame after being dispatched.
struct InputActionHolder
{
    InputAction action;

    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;

    union
    {
        bool fixedDispatchDone = false;
    };

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
