#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Log
{
/// \brief Supported logging levels.
enum class Level
{
    VERBOSE = 0u,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL_ERROR,
};

namespace Sinks
{
/// \brief Common sink fields.
struct Base
{
    /// \brief Only messages with equal or greater level are printed.
    Level minimumAcceptedLevel = Level::VERBOSE;

    /// \brief Messages with this level or greater inform logger that flush should be done right now.
    Level minimumFlushLevel = Level::VERBOSE;
};

/// \brief Prints messages to standard output.
struct StandardOut final : public Base
{
};

/// \brief Prints messages to standard error output.
struct StandardError final : public Base
{
};

/// \brief Prints messages to file with given name.
struct File final : public Base
{
    /// \brief Name of target file.
    std::string fileName;

    /// \brief If true, file content will be cleared before writing any messages.
    bool overwrite = true;
};
} // namespace Sinks

/// \brief Variant, that contains configuration for any available sink.
using Sink = std::variant<Sinks::StandardOut, Sinks::StandardError, Sinks::File>;

/// \brief Allows logging messages to given sequence of sinks.
class Logger final
{
public:
    /// \brief Constructs logger, that prints messages to given sinks.
    Logger (const std::vector<Sink> &_sinks) noexcept;

    /// Loggers are not guaranteed to safely share some sinks, for example Sinks::File.
    Logger (const Logger &_logger) = default;

    Logger (Logger &&_logger) noexcept;

    ~Logger () noexcept;

    /// \brief Logs message with given level. Thread safe.
    void Log (Level _level, const std::string &_message) noexcept;

    /// It looks counter intuitive to move assign loggers.
    EMERGENCE_DELETE_ASSIGNMENT (Logger);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 5u);
};

namespace GlobalLogger
{
/// \brief Initializes shared global logger instance.
void Init (const std::vector<Sink> &_sinks) noexcept;

/// \return Shared global logger or nullptr if it's not initialized.
Logger *Get () noexcept;
}; // namespace GlobalLogger
} // namespace Emergence::Log
