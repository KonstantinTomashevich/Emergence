#pragma once

#include <Reference/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Reference::Test
{
Scenario ConstructAndDestructSingle ();

Scenario ConstructAndDestructMultiple ();

Scenario MoveChain ();

Scenario MoveCopy ();

Scenario CopyMultiple ();

Scenario CopyAssignMultiple ();

Scenario MoveAssignChain ();
} // namespace Emergence::Reference::Test

#define REGISTER_REFERENCE_TEST(Driver, TestName)                                                                      \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Reference::Test::TestName ());                                                              \
    }

#define REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT(Driver)                                                        \
    REGISTER_REFERENCE_TEST (Driver, ConstructAndDestructSingle)                                                       \
    REGISTER_REFERENCE_TEST (Driver, ConstructAndDestructMultiple)                                                     \
    REGISTER_REFERENCE_TEST (Driver, MoveChain)                                                                        \
    REGISTER_REFERENCE_TEST (Driver, MoveCopy)                                                                         \
    REGISTER_REFERENCE_TEST (Driver, CopyMultiple)

#define REGISTER_ALL_REFERENCE_TESTS(Driver)                                                                           \
    REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (Driver)                                                           \
    REGISTER_REFERENCE_TEST (Driver, CopyAssignMultiple)                                                               \
    REGISTER_REFERENCE_TEST (Driver, MoveAssignChain)
