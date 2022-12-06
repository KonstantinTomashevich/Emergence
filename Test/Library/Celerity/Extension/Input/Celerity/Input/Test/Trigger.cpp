#include <Celerity/Input/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (Trigger)

TEST_CASE (KeyDownOnState)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                              {200u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{}, {}, {}, {}, 0u, {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             0u,
             KeyState::DOWN,
             KeyTriggerType::ON_STATE,
             InputActionDispatchType::NORMAL},
        });
}

TEST_CASE (KeyDownOnStateChanged)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                              {200u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {}}}},
            NormalUpdate {{}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             0u,
             KeyState::DOWN,
             KeyTriggerType::ON_STATE_CHANGED,
             InputActionDispatchType::NORMAL},
        });
}

TEST_CASE (KeyUpOnStateChanged)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                              {200u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {1u, 1u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {}}}},
            NormalUpdate {{}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             0u,
             KeyState::UP,
             KeyTriggerType::ON_STATE_CHANGED,
             InputActionDispatchType::NORMAL},
        });
}

TEST_CASE (KeyDownOnStateChangedQualifiersFound)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 1u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             1u,
             KeyState::DOWN,
             KeyTriggerType::ON_STATE_CHANGED,
             InputActionDispatchType::NORMAL},
        });
}

TEST_CASE (KeyDownOnStateChangedQualifiersMissing)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             1u,
             KeyState::DOWN,
             KeyTriggerType::ON_STATE_CHANGED,
             InputActionDispatchType::NORMAL},
        });
}

TEST_CASE (KeyDownOnStateQualifiers)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}}, {}, {}, {}, 1u, {{0u, {}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 1u, Emergence::Celerity::KeyState::DOWN, false}},
                          },
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{}, {}, {}, {}, 0u, {{0u, {}}}},
            NormalUpdate {{}, {}, {}, {}, 1u, {{0u, {{"TestGroup"_us, "TestAction1"_us}}}}},
            NormalUpdate {{},
                          {},
                          {},
                          {
                              {100u, KeyboardEvent {0u, 0u, 0u, Emergence::Celerity::KeyState::UP, false}},
                          },
                          0u,
                          {{0u, {}}}},
        },
        {
            {{"TestGroup"_us, "TestAction1"_us},
             0u,
             1u,
             KeyState::DOWN,
             KeyTriggerType::ON_STATE,
             InputActionDispatchType::NORMAL},
        });
}

END_SUITE
