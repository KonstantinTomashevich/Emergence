#pragma once

#include <Input/InputAction.hpp>

#include <Shared/Constants.hpp>

#include <SyntaxSugar/InplaceVector.hpp>

enum class KeyState
{
    JUST_PRESSED = 0u,
    PRESSED,
    RELEASED,
};

struct KeyRequirement final
{
    std::int32_t key = 0;
    std::uint8_t justPressed = 0u;
    KeyState currentKeyState = KeyState::RELEASED;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId key;
        Emergence::StandardLayout::FieldId justPressed;
        Emergence::StandardLayout::FieldId currentKeyState;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct KeyboardActionTrigger final
{
    constexpr static const std::size_t MAX_KEY_REQUIREMENTS = 3u;

    InputAction action;
    Emergence::InplaceVector<KeyRequirement, MAX_KEY_REQUIREMENTS> requirements;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId action;
        std::array<Emergence::StandardLayout::FieldId, MAX_KEY_REQUIREMENTS> requirements;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct InputSubscription final
{
    Emergence::String::ConstReference group;
    std::uint64_t listenerId = INVALID_OBJECT_ID;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId group;
        Emergence::StandardLayout::FieldId listenerId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct InputMapping final
{
    constexpr static const std::size_t MAX_KEYBOARD_TRIGGERS = 16u;
    constexpr static const std::size_t MAX_SUBSCRIPTIONS = 16u;
    constexpr static const std::size_t ACTION_BUFFER_SIZE = 64u;

    Emergence::InplaceVector<KeyboardActionTrigger, MAX_KEYBOARD_TRIGGERS> keyboardTriggers;
    Emergence::InplaceVector<InputSubscription, MAX_SUBSCRIPTIONS> subscriptions;
    Emergence::InplaceVector<InputAction, ACTION_BUFFER_SIZE> currentFrameActions;

    struct Reflection final
    {
        std::array<Emergence::StandardLayout::FieldId, MAX_KEYBOARD_TRIGGERS> keyboardTriggers;
        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> subscriptions;
        std::array<Emergence::StandardLayout::FieldId, ACTION_BUFFER_SIZE> currentFrameActions;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
