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

/// \brief Check that given values are equal and log on failure.
#define CHECK_EQUAL(...) IMPLEMENTATION_CHECK_EQUAL (__VA_ARGS__)

/// \brief Check that given values are equal and abort test on failure.
#define REQUIRE_EQUAL(...) IMPLEMENTATION_REQUIRE_EQUAL (__VA_ARGS__)

/// \brief Check that given values are not equal and log on failure.
#define CHECK_NOT_EQUAL(...) IMPLEMENTATION_CHECK_NOT_EQUAL (__VA_ARGS__)

/// \brief Check that given values are not equal and abort test on failure.
#define REQUIRE_NOT_EQUAL(...) IMPLEMENTATION_REQUIRE_NOT_EQUAL (__VA_ARGS__)

/// \brief Check assertion and log given message on failure.
#define CHECK_WITH_MESSAGE(assertion, ...) IMPLEMENTATION_CHECK_WITH_MESSAGE (assertion, __VA_ARGS__)

/// \brief Check assertion, log given message and abort test on failure.
#define REQUIRE_WITH_MESSAGE(assertion, ...) IMPLEMENTATION_REQUIRE_WITH_MESSAGE (assertion, __VA_ARGS__)

/// \brief Log debug message from test.
#define LOG(...) IMPLEMENTATION_LOG (__VA_ARGS__)
