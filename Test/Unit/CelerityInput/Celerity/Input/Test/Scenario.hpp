#pragma once

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test
{
struct ActionExpectation final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Container::Vector<InputAction> actions;
};

struct SubscriptionInfo final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    Memory::UniqueString groupId;
};

struct ExternalAction final
{
    InputAction action;
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;
};

struct NormalUpdate final
{
    Container::Vector<SubscriptionInfo> subscriptionsToAdd;
    Container::Vector<SubscriptionInfo> subscriptionsToRemove;
    Container::Vector<ExternalAction> externalActions;
    Container::Vector<InputStorage::InputEvent> inputEvents;
    InputStorage::QualifiersMask currentQualifiersMask = 0u;
    Container::Vector<ActionExpectation> expectations;
};

struct FixedUpdate final
{
    Container::Vector<SubscriptionInfo> subscriptionsToAdd;
    Container::Vector<SubscriptionInfo> subscriptionsToRemove;
    Container::Vector<ActionExpectation> expectations;
};

using Update = Container::Variant<NormalUpdate, FixedUpdate>;

struct KeyTriggerSetup final
{
    InputAction actionToSend;
    InputStorage::ScanCode triggerCode = 0u;
    InputStorage::QualifiersMask expectedQualifiers = 0u;
    InputStorage::KeyState triggerTargetState = InputStorage::KeyState::DOWN;
    KeyTriggerType triggerType = KeyTriggerType::ON_STATE;
    InputActionDispatchType dispatchType = InputActionDispatchType::NORMAL;
};

void ExecuteScenario (const Container::Vector<Update> &_updates,
                      Container::Vector<KeyTriggerSetup> _keyTriggers) noexcept;
} // namespace Emergence::Celerity::Test
