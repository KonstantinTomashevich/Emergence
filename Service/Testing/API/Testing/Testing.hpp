#pragma once

#include <Testing/Implementation/Testing.hpp>

#define TEST_CASE(name) IMPLEMENTATION_TEST_CASE (name)

#define BEGIN_SUITE(name) IMPLEMENTATION_BEGIN_SUITE (name)

#define END_SUITE IMPLEMENTATION_END_SUITE

#define CHECK(...) IMPLEMENTATION_CHECK (__VA_ARGS__)

#define REQUIRE(...) IMPLEMENTATION_REQUIRE (__VA_ARGS__)

#define INFO(...) IMPLEMENTATION_INFO (__VA_ARGS__)