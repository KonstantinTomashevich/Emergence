// We don't check fmt exceptions here.
#define FMT_EXCEPTIONS 0

#include <atomic>
#include <cassert>
#include <memory>

#include <Container/Optional.hpp>

#include <Log/Log.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence
{
namespace Log
{
class LoggerImplementation final
{
public:
    LoggerImplementation (Level _forceFlushOn, const Container::Vector<Sink> &_sinks) noexcept;

    LoggerImplementation (const LoggerImplementation &_other) = delete;

    LoggerImplementation (LoggerImplementation &&_other) noexcept;

    ~LoggerImplementation () noexcept = default;

    void Log (Level _level, const Container::String &_message) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (LoggerImplementation);

private:
    std::atomic_flag locked;
    spdlog::logger logger;
};

static spdlog::level::level_enum ToSPDLogLevel (Level _level)
{
    switch (_level)
    {
    case Level::VERBOSE:
        return spdlog::level::trace;
    case Level::DEBUG:
        return spdlog::level::debug;
    case Level::INFO:
        return spdlog::level::info;
    case Level::WARNING:
        return spdlog::level::warn;
    case Level::ERROR:
        return spdlog::level::err;
    case Level::CRITICAL_ERROR:
        return spdlog::level::critical;
    }

    assert (false);
    return spdlog::level::critical;
}

LoggerImplementation::LoggerImplementation (Level _forceFlushOn, const Container::Vector<Sink> &_sinks) noexcept
    // Loggers do not share sinks (except stdout and stderr), therefore there is no need for unique names.
    : logger (spdlog::logger ("Logger"))
{
    // Logger should accept all messages: only sinks should filter messages by level.
    logger.set_level (ToSPDLogLevel (Level::VERBOSE));
    logger.flush_on (ToSPDLogLevel (_forceFlushOn));

    for (const Sink &sink : _sinks)
    {
        std::visit (
            [this] (const auto &_config)
            {
                auto addSink = [this, &_config] (auto _sink)
                {
                    _sink->set_level (ToSPDLogLevel (_config.minimumAcceptedLevel));
                    logger.sinks ().emplace_back (std::move (_sink));
                };

                // We use single threaded sinks here, because we can more efficiently
                // protect them from multithread access using ::locked.
                //
                // Unfortunately, there is no easy way to profile these allocations due to spdlog architecture.
                // But this allocations are small and therefore we can just ignore them.
                if constexpr (std::is_same_v<Sinks::StandardOut, std::decay_t<decltype (_config)>>)
                {
                    addSink (std::make_shared<spdlog::sinks::stdout_color_sink_st> ());
                }
                else if constexpr (std::is_same_v<Sinks::StandardError, std::decay_t<decltype (_config)>>)
                {
                    addSink (std::make_shared<spdlog::sinks::stderr_color_sink_st> ());
                }
                else if constexpr (std::is_same_v<Sinks::File, std::decay_t<decltype (_config)>>)
                {
                    addSink (std::make_shared<spdlog::sinks::basic_file_sink_st> (_config.fileName.c_str (),
                                                                                  _config.overwrite));
                }
            },
            sink);
    }
}

LoggerImplementation::LoggerImplementation (LoggerImplementation &&_other) noexcept : logger (std::move (_other.logger))
{
    // Assert that there is no active logging operation.
    assert (!_other.locked.test ());
}

void LoggerImplementation::Log (Level _level, const Container::String &_message) noexcept
{
    spdlog::level::level_enum level = ToSPDLogLevel (_level);

    // Usually there is no sense to print ton of logs in production, therefore it's better to use flag based spin lock.
    while (locked.test_and_set (std::memory_order_acquire))
    {
        std::this_thread::yield ();
    }

    logger.log (level, _message);
    locked.clear (std::memory_order_release);
}

Logger::Logger (Level _forceFlushOn, const Container::Vector<Sink> &_sinks) noexcept
{
    new (&data) LoggerImplementation (_forceFlushOn, _sinks);
}

Logger::Logger (Logger &&_other) noexcept
{
    new (&data) LoggerImplementation (std::move (block_cast<LoggerImplementation> (_other.data)));
}

Logger::~Logger () noexcept
{
    block_cast<LoggerImplementation> (data).~LoggerImplementation ();
}

void Logger::Log (Level _level, const Container::String &_message) noexcept
{
    block_cast<LoggerImplementation> (data).Log (_level, _message);
}

namespace GlobalLogger
{
static Container::Optional<Logger> globalLogger;

void Init (Level _forceFlushOn, const Container::Vector<Sink> &_sinks) noexcept
{
    assert (!globalLogger);
    globalLogger.emplace (_forceFlushOn, _sinks);
}

void Log (Level _level, const Container::String &_message) noexcept
{
    if (!globalLogger)
    {
        Init ();
    }

    globalLogger->Log (_level, _message);
}
} // namespace GlobalLogger
} // namespace Log

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Log::Sink>::Get () noexcept
{
    return Profiler::AllocationGroup {UniqueString {"LogSink"}};
}
} // namespace Memory
} // namespace Emergence
