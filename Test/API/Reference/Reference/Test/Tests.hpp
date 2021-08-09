#pragma once

#include <Reference/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Reference::Test
{
Scenario ConstructAndDestructMultiple ();

Scenario MoveChain ();

Scenario MoveCopy ();

Scenario CopyMultiple ();

Scenario CopyAssignMultiple ();

Scenario MoveAssignChain ();
} // namespace Emergence::Reference::Test

#define REGISTER_REFERENCE_TEST(Driver, TestName)                              \
TEST_CASE (TestName)                                                           \
{                                                                              \
    Driver (Emergence::Reference::Test::TestName ());                          \
}                                                                              \
