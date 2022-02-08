#pragma once

#include <Celerity/InputAccumulator.hpp>

#include <Container/InplaceVector.hpp>

#include <Input/InputAction.hpp>

#include <Shared/Constants.hpp>

/// \brief Produces persistent actions when key is down/up.
/// \details Keys are identified by scan codes for layout independence.
struct KeyStateTrigger final
{
    InputAction action;
    Emergence::Celerity::ScanCode scan;
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
    Emergence::Celerity::ScanCode scan;
    bool pressed;
    Emergence::Celerity::QualifiersMask qualifiers;

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
        std::array<Emergence::StandardLayout::FieldId, MAX_TRIGGERS> keyStateTriggers;
        std::array<Emergence::StandardLayout::FieldId, MAX_TRIGGERS> keyStateChangedTriggers;

        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> normalSubscriptions;
        std::array<Emergence::StandardLayout::FieldId, MAX_SUBSCRIPTIONS> fixedSubscriptions;

        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> normalActionsBuffer;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedInstantActionsBuffer;
        std::array<Emergence::StandardLayout::FieldId, MAX_ACTIONS_IN_QUEUE> fixedPersistentActionsBuffer;

        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
