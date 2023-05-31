#pragma once

#include <Container/Optional.hpp>
#include <Container/String.hpp>
#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

namespace Emergence::Log::Test
{
constexpr const char *GLOBAL_LOG_FILE_NAME = "Test.log";
constexpr const char *CUSTOM_LOG_FILE_NAME = "TestCustom.log";

constexpr std::size_t THREAD_COUNT = 4u;
constexpr std::size_t ITERATIONS = 1000u;

constexpr std::size_t VERBOSE_FREQUENCY = 1u;
constexpr std::size_t DEBUG_FREQUENCY = 3u;
constexpr std::size_t INFO_FREQUENCY = 10u;
constexpr std::size_t WARNING_FREQUENCY = 20u;
constexpr std::size_t ERROR_FREQUENCY = 30u;
constexpr std::size_t CRITICAL_ERROR_FREQUENCY = 100u;

constexpr Level STANDARD_OUT_MINIMUM_LEVEL = Level::VERBOSE;
constexpr Level STANDARD_ERROR_MINIMUM_LEVEL = Level::ERROR;
constexpr Level GLOBAL_FILE_MINIMUM_LEVEL = Level::INFO;
constexpr Level CUSTOM_FILE_MINIMUM_LEVEL = Level::DEBUG;

constexpr const char *VERBOSE_MESSAGE = "Verbose message";
constexpr const char *DEBUG_MESSAGE = "Debug message";
constexpr const char *INFO_MESSAGE = "Info message";
constexpr const char *WARNING_MESSAGE = "Warning message";
constexpr const char *ERROR_MESSAGE = "Error message";
constexpr const char *CRITICAL_ERROR_MESSAGE = "Critical error message";

void ConstructMessage (Container::StringBuilder &_builder, std::size_t _index, const char *_message);

Container::Optional<std::size_t> ExtractThreadIndexFromMessage (const Container::String &_message);
} // namespace Emergence::Log::Test
