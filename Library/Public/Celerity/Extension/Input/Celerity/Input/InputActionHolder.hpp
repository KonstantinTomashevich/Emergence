#pragma once

#include <Celerity/Input/InputAction.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
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
