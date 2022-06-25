#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/InplaceVector.hpp>

#include <Input/InputAction.hpp>

/// \brief Code of the key, localized to user keyboard layout.
using KeyCode = std::int32_t;

/// \brief Physical layout-independent code of the key.
using ScanCode = std::int32_t;

/// \brief Represents which qualifier keys are active.
using QualifiersMask = std::uint32_t;

/// \brief Produces persistent actions when key is down/up.
/// \details Keys are identified by scan codes for layout independence.
struct KeyStateTrigger final
{
    InputAction action;
    ScanCode scan;
    bool down;

    bool isDownNow = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId action;
        Emergence::StandardLayout::FieldId scan;
        Emergence::StandardLayout::FieldId down;
        Emergence::StandardLayout::FieldId isDownNow;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Produces instant actions when key is pressed/released.
/// \details Keys are identified by scan codes for layout independence.
struct KeyStateChangedTrigger final
{
    InputAction action;
    ScanCode scan;
    bool pressed;
    QualifiersMask qualifiers;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId action;
        Emergence::StandardLayout::FieldId scan;
        Emergence::StandardLayout::FieldId pressed;
        Emergence::StandardLayout::FieldId qualifiers;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

// TODO: Bindings for parametrized input like movement. For example, specify how keys add up to parameters.

struct InputSubscription final
{
    Emergence::Memory::UniqueString group;
    Emergence::Celerity::UniqueId listenerId = Emergence::Celerity::INVALID_UNIQUE_ID;

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
    constexpr static const std::size_t MAX_TRIGGERS = 32u;
    constexpr static const std::size_t MAX_SUBSCRIPTIONS = 8u;
    constexpr static const std::size_t MAX_ACTIONS_IN_QUEUE = 32u;

    using KeyStateChangedTriggerVector = Emergence::Container::InplaceVector<KeyStateChangedTrigger, MAX_TRIGGERS>;
    using KeyStateTriggerVector = Emergence::Container::InplaceVector<KeyStateTrigger, MAX_TRIGGERS>;
    using SubscriptionVector = Emergence::Container::InplaceVector<InputSubscription, MAX_SUBSCRIPTIONS>;
    using ActionsBuffer = Emergence::Container::InplaceVector<InputAction, MAX_ACTIONS_IN_QUEUE>;

    void UnsubscribeNormal (std::uintptr_t _listenerId) noexcept;

    void UnsubscribeFixed (std::uintptr_t _listenerId) noexcept;

    void UnsubscribeGroup (Emergence::Memory::UniqueString _group) noexcept;

    KeyStateTriggerVector keyStateTriggers;
    KeyStateChangedTriggerVector keyStateChangedTriggers;

    SubscriptionVector normalSubscriptions;
    SubscriptionVector fixedSubscriptions;

    ActionsBuffer normalActionsBuffer;
    ActionsBuffer fixedInstantActionsBuffer;
    ActionsBuffer fixedPersistentActionsBuffer;

    /// \details External code can push actions to any buffer, including ::fixedPersistentActionsBuffer,
    ///          which should be cleared during normal update that goes right after fixed update.
    ///          Between fixed update end and input dispatch begin external features like UI and replays
    ///          are allowed to push their events, and these events should not be lost by accident.
    ///          Therefore, we need to either clear ::fixedPersistentActionsBuffer is separate task before
    ///          everything or clear it partially using this counter to separate old and new events. Counter
    ///          was selected because additional task increases pipeline complexity.
    std::size_t accumulatedPersistentActionsForFixedUpdate = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId keyStateTriggersBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_TRIGGERS> keyStateTriggers;

        Emergence::StandardLayout::FieldId keyStateChangedTriggersBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_TRIGGERS> keyStateChangedTriggers;

        Emergence::StandardLayout::FieldId normalSubscriptionsBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> normalSubscriptions;

        Emergence::StandardLayout::FieldId fixedSubscriptionsBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> fixedSubscriptions;

        Emergence::StandardLayout::FieldId normalActionsBufferBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> normalActionsBuffer;

        Emergence::StandardLayout::FieldId fixedInstantActionsBufferBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedInstantActionsBuffer;

        Emergence::StandardLayout::FieldId fixedPersistentActionsBufferBlock;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedPersistentActionsBuffer;

        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
