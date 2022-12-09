#include <Celerity/Input/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

BEGIN_SUITE (Dispatch)

TEST_CASE (NormalExternalDispatch)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::NORMAL},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::NORMAL},
                          },
                          {},
                          0u,
                          {{0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}}}},
            FixedUpdate {{}, {}, {{0u, {}}}},
        },
        {});
}

TEST_CASE (FixedInstantExternalDispatch)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::FIXED_INSTANT},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::FIXED_INSTANT},
                          },
                          {},
                          0u,
                          {{0u, {}}}},
            FixedUpdate {{}, {}, {{0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}}}},
            FixedUpdate {{}, {}, {{0u, {}}}},
        },
        {});
}

TEST_CASE (FixedPersistentExternalDispatch)
{
    ExecuteScenario (
        {
            NormalUpdate {{{0u, "TestGroup"_us}},
                          {},
                          {
                              {{"TestGroup"_us, "TestAction1"_us}, InputActionDispatchType::FIXED_PERSISTENT},
                              {{"TestGroup"_us, "TestAction2"_us}, InputActionDispatchType::FIXED_PERSISTENT},
                          },
                          {},
                          0u,
                          {{0u, {}}}},
            FixedUpdate {{}, {}, {{0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}}}},
            FixedUpdate {{}, {}, {{0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}}}},
            FixedUpdate {{}, {}, {{0u, {{"TestGroup"_us, "TestAction1"_us}, {"TestGroup"_us, "TestAction2"_us}}}}},
            NormalUpdate {{}, {}, {}, {}, 0u, {{0u, {}}}},
            FixedUpdate {{}, {}, {{0u, {}}}},
        },
        {});
}

END_SUITE
