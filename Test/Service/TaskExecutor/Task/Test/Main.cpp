#include <Task/Test/Executor.hpp>

#include <Testing/SetupMain.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool EXECUTOR_TEST_INCLUDE_MARKER = Emergence::Task::Test::ExecutorTestIncludeMarker ();
