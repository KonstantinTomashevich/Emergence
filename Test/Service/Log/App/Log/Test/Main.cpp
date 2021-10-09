#include <thread>
#include <vector>

#include <Log/Log.hpp>
#include <Log/Test/Shared.hpp>

using namespace Emergence::Log;

void ThreadFunction (Logger &_logger, std::size_t _index)
{
    auto log = [&_logger, _index] (Level _level, const char *_message)
    {
        std::string message = Test::AddThreadIndexToMessage (_index, _message);
        _logger.Log (_level, message);
        GlobalLogger::Log (_level, message);
    };

    for (std::size_t iteration = 1u; iteration <= Test::ITERATIONS; ++iteration)
    {
        if (iteration % Test::VERBOSE_FREQUENCY == 0u)
        {
            log (Level::VERBOSE, Test::VERBOSE_MESSAGE);
        }

        if (iteration % Test::DEBUG_FREQUENCY == 0u)
        {
            log (Level::DEBUG, Test::DEBUG_MESSAGE);
        }

        if (iteration % Test::INFO_FREQUENCY == 0u)
        {
            log (Level::INFO, Test::INFO_MESSAGE);
        }

        if (iteration % Test::WARNING_FREQUENCY == 0u)
        {
            log (Level::WARNING, Test::WARNING_MESSAGE);
        }

        if (iteration % Test::ERROR_FREQUENCY == 0u)
        {
            log (Level::ERROR, Test::ERROR_MESSAGE);
        }

        if (iteration % Test::CRITICAL_ERROR_FREQUENCY == 0u)
        {
            log (Level::CRITICAL_ERROR, Test::CRITICAL_ERROR_MESSAGE);
        }
    }
}

int main (int /* unused */, char ** /* unused */)
{
    GlobalLogger::Init (Level::ERROR, {
                                          Sinks::StandardOut {{Test::STANDARD_OUT_MINIMUM_LEVEL}},
                                          Sinks::StandardError {{Test::STANDARD_ERROR_MINIMUM_LEVEL}},
                                          Sinks::File {{Test::GLOBAL_FILE_MINIMUM_LEVEL}, Test::GLOBAL_LOG_FILE_NAME},
                                      });

    Logger customLogger {Level::ERROR,
                         {
                             Sinks::File {{Test::CUSTOM_FILE_MINIMUM_LEVEL}, Test::CUSTOM_LOG_FILE_NAME},
                         }};

    std::vector<std::thread> threads;
    threads.reserve (Test::THREAD_COUNT);

    for (std::size_t index = 0u; index < Test::THREAD_COUNT; ++index)
    {
        threads.emplace_back (ThreadFunction, std::ref (customLogger), index);
    }

    for (std::size_t index = 0u; index < Test::THREAD_COUNT; ++index)
    {
        threads[index].join ();
    }

    return 0;
}
