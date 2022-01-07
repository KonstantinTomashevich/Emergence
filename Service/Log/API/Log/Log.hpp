#pragma once

#include <cstdint>
#include <variant>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <Memory/Heap.hpp>

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
    Container::String fileName;

    /// \brief If true, file content will be cleared before writing any messages.
    bool overwrite = true;
};
} // namespace Sinks

/// \brief Variant, that contains configuration for any available sink type.
using Sink = std::variant<Sinks::StandardOut, Sinks::StandardError, Sinks::File>;

/// \brief Allows logging messages to given set of sinks.
class Logger final
{
public:
    /// \brief Constructs logger, that prints messages to given sinks.
    /// \param _forceFlushOn After message with this level or above appears, all messages will be flushed right away.
    /// \param _sinks List of sinks for this logger.
    Logger (Level _forceFlushOn, const Container::Vector<Sink> &_sinks) noexcept;

    /// It looks counter intuitive to copy loggers.
    Logger (const Logger &_other) = delete;

    Logger (Logger &&_other) noexcept;

    ~Logger () noexcept;

    // TODO: Use raw pointers instead of strings?
    // TODO: Some API for string appending?

    /// \brief Logs message with given level. Thread safe.
    /// \details Not guaranteed to flush right away if _level is lower than force flush level.
    ///          If there is no messages with force flush level, logger is guaranteed to flush messages periodically.
    void Log (Level _level, const Container::String &_message) noexcept;

    /// It looks counter intuitive to assign loggers.
    EMERGENCE_DELETE_ASSIGNMENT (Logger);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 36u);
};

namespace GlobalLogger
{
/// \brief Initializes shared global logger instance.
/// \invariant Should not be called more than once.
void Init (Level _forceFlushOn = Level::ERROR,
           const Container::Vector<Sink> &_sinks = {Sinks::StandardOut {{}}}) noexcept;

/// \brief Executes Logger::Log using global logger instance.
/// \details If ::Init was not called previously, it would be called with default arguments.
void Log (Level _level, const Container::String &_message) noexcept;
}; // namespace GlobalLogger
} // namespace Emergence::Log

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Emergence::Log::Sink)
