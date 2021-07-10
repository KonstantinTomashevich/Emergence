#pragma once

#include <Testing/Implementation/Testing.hpp>

/// \brief Begin test suite scope.
#define BEGIN_SUITE(name) IMPLEMENTATION_BEGIN_SUITE (name)

/// \brief End test suite scope.
#define END_SUITE IMPLEMENTATION_END_SUITE

/// \brief Register test case. Must be followed by test function body.
#define TEST_CASE(name) IMPLEMENTATION_TEST_CASE (name)

/// \brief Check assertion and log on failure.
#define CHECK(...) IMPLEMENTATION_CHECK (__VA_ARGS__)

/// \brief Check assertion and abort test on failure.
#define REQUIRE(...) IMPLEMENTATION_REQUIRE (__VA_ARGS__)

/// \brief Log debug message from test.
#define LOG(...) IMPLEMENTATION_LOG (__VA_ARGS__)