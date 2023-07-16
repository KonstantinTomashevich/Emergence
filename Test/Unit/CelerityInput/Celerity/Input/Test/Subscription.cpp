#include <Celerity/Input/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (Subscription)

TEST_CASE (NoSubscription)
{
    ExecuteScenario (
        {
            NormalUpdate {{},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {{0u, {}}}},
        },
        {});
}

TEST_CASE (WrongGroup)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroupX"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {{0u, {}}}},
        },
        {});
}

TEST_CASE (TwoSubscriptions)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}, {1u, "TestGroup"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {
                              {0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}},
                              {1u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}},
                          }},
        },
        {});
}

TEST_CASE (SubscriptionRemove)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}, {1u, "TestGroup"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {
                              {0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}},
                              {1u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}},
                          }},
            NormalUpdate {{},
                          {{0u, "TestGroup"_us}},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {
                              {0u, {}},
                              {1u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}},
                          }},
        },
        {});
}

END_SUITE
