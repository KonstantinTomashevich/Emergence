#pragma once

#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/Input/InputAction.hpp>

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class InputControlType : uint8_t
{
    TEXT = 0u,
    INT,
    FLOAT
};

struct InputControl final
{
    static constexpr size_t MAX_TEXT_LENGTH = 64u;

    UniqueId nodeId = INVALID_UNIQUE_ID;
    InputControlType type = InputControlType::TEXT;
    Container::Utf8String label;

    InputAction onChangedAction;
    InputActionDispatchType onChangedActionDispatch = InputActionDispatchType::NORMAL;

    union
    {
        std::array<char, MAX_TEXT_LENGTH> utf8TextValue;
        int32_t intValue;
        float floatValue;
    };

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId type;

        StandardLayout::FieldId onChangedAction;
        StandardLayout::FieldId onChangedActionDispatch;

        StandardLayout::FieldId utf8TextValue;
        StandardLayout::FieldId intValue;
        StandardLayout::FieldId floatValue;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
