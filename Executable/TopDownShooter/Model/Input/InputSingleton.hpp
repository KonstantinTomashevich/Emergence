#pragma once

#include <Container/InplaceVector.hpp>

#include <Input/InputAction.hpp>

#include <Shared/Constants.hpp>

using KeyCode = std::int32_t;

using QualifiersMask = std::uint32_t;

struct KeyboardActionTrigger final
{
    constexpr static const std::size_t MAX_KEY_REQUIREMENTS = 3u;

    InputAction action;
    Emergence::Container::InplaceVector<KeyCode, MAX_KEY_REQUIREMENTS> keys;
    QualifiersMask qualifiers;

    uint32_t keysState = 0u;
    static_assert (sizeof (keysState) * 8u >= MAX_KEY_REQUIREMENTS);

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId action;
        std::array<Emergence::StandardLayout::FieldId, MAX_KEY_REQUIREMENTS> keys;
        Emergence::StandardLayout::FieldId qualifiers;
        Emergence::StandardLayout::FieldId keysState;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

// TODO: Bindings for parametrized input like movement. For example, specify how keys add up to parameters.

struct InputSubscription final
{
    Emergence::Memory::UniqueString group;
    std::uintptr_t listenerId = INVALID_OBJECT_ID;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId group;
        Emergence::StandardLayout::FieldId listenerId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct InputSingleton final
{
public:
    constexpr static const std::size_t MAX_KEYBOARD_TRIGGERS = 32u;
    constexpr static const std::size_t MAX_SUBSCRIPTIONS = 8u;
    constexpr static const std::size_t MAX_ACTIONS_IN_QUEUE = 32u;

    using KeyboardTriggerVector = Emergence::Container::InplaceVector<KeyboardActionTrigger, MAX_KEYBOARD_TRIGGERS>;
    using SubscriptionVector = Emergence::Container::InplaceVector<InputSubscription, MAX_SUBSCRIPTIONS>;
    using ActionsBuffer = Emergence::Container::InplaceVector<InputAction, MAX_ACTIONS_IN_QUEUE>;

    void UnsubscribeNormal (std::uintptr_t _listenerId) noexcept;

    void UnsubscribeFixed (std::uintptr_t _listenerId) noexcept;

    void UnsubscribeGroup (Emergence::Memory::UniqueString _group) noexcept;

    KeyboardTriggerVector keyboardInstantTriggers;
    KeyboardTriggerVector keyboardPersistentTriggers;

    SubscriptionVector normalSubscriptions;
    SubscriptionVector fixedSubscriptions;

    ActionsBuffer normalActionsBuffer;
    ActionsBuffer fixedInstantActionsBuffer;
    ActionsBuffer fixedPersistentActionsBuffer;

    /// \details External code can push actions to any buffer, including ::fixedPersistentActionsBuffer,
    ///          which should be cleared during normal update that goes right after fixed update.
    ///          But between fixed update end and input dispatch begin external features like UI and replays
    ///          are allowed to push their events, and these events should not be lost by accident.
    ///          Therefore, we need to either clear ::fixedPersistentActionsBuffer is separate task before
    ///          everything or clear it partially using this counter to distinct old and new events. Counter
    ///          was selected because additional task increases pipeline complexity.
    std::size_t accumulatedPersistentActionsForFixedUpdate = 0u;

    struct Reflection final
    {
        std::array<Emergence::StandardLayout::FieldId, MAX_KEYBOARD_TRIGGERS> keyboardInstantTriggers;
        std::array<Emergence::StandardLayout::FieldId, MAX_KEYBOARD_TRIGGERS> keyboardPersistentTriggers;

        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> normalSubscriptions;
        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> fixedSubscriptions;

        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> normalActionsBuffer;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedInstantActionsBuffer;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedPersistentActionsBuffer;

        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
