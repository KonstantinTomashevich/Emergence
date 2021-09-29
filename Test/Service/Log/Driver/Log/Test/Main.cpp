#include <fstream>
#include <iostream>

#include <Log/Test/Shared.hpp>

constexpr int RESULT_OK = 0u;
constexpr int RESULT_ERROR = 1u;

constexpr int ARGUMENT_LOGGER = 1u;

constexpr const char *STDOUT_FILE = "StdOut.log";
constexpr const char *STDERR_FILE = "StdErr.log";

using namespace Emergence::Log;

static std::vector<Level> ExpectedSequenceForOneThread (Level _minimumLevel)
{
    std::vector<Level> sequence;
    for (std::size_t iteration = 1u; iteration <= Test::ITERATIONS; ++iteration)
    {
        if (Level::VERBOSE >= _minimumLevel && iteration % Test::VERBOSE_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::VERBOSE);
        }

        if (Level::DEBUG >= _minimumLevel && iteration % Test::DEBUG_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::DEBUG);
        }

        if (Level::INFO >= _minimumLevel && iteration % Test::INFO_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::INFO);
        }

        if (Level::WARNING >= _minimumLevel && iteration % Test::WARNING_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::WARNING);
        }

        if (Level::ERROR >= _minimumLevel && iteration % Test::ERROR_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::ERROR);
        }

        if (Level::CRITICAL_ERROR >= _minimumLevel && iteration % Test::CRITICAL_ERROR_FREQUENCY == 0u)
        {
            sequence.emplace_back (Level::CRITICAL_ERROR);
        }
    }

    return sequence;
}

static const char *LevelToMessage (Level _level)
{
    switch (_level)
    {
    case Level::VERBOSE:
        return Test::VERBOSE_MESSAGE;
    case Level::DEBUG:
        return Test::DEBUG_MESSAGE;
    case Level::INFO:
        return Test::INFO_MESSAGE;
    case Level::WARNING:
        return Test::WARNING_MESSAGE;
    case Level::ERROR:
        return Test::ERROR_MESSAGE;
    case Level::CRITICAL_ERROR:
        return Test::CRITICAL_ERROR_MESSAGE;
    }

    return "Unknown level message.";
}

std::size_t CheckLog (std::istream &_input, Level _minimumLevel)
{
    std::vector<Level> expectedSequence = ExpectedSequenceForOneThread (_minimumLevel);
    std::array<std::size_t, Test::THREAD_COUNT> messagesIndexPerThread {0u};
    // If there is an error in thread log, it's better to skip all next messages for this thread.
    std::array<bool, Test::THREAD_COUNT> skipThread {false};

    std::size_t lineNumber = 0u;
    std::size_t errorsDetected = 0u;
    std::string line;

    while (std::getline (_input, line))
    {
        ++lineNumber;
        std::optional<std::size_t> threadIndex = Test::ExtractThreadIndexFromMessage (line);

        if (!threadIndex)
        {
            ++errorsDetected;
            std::cout << "ERROR on line " << lineNumber << ": Unable to extract thread index. Line: \"" << line << "\"."
                      << std::endl;
            continue;
        }

        if (threadIndex.value () >= Test::THREAD_COUNT)
        {
            ++errorsDetected;
            std::cout << "ERROR: Thread index " << threadIndex.value () << " is too big!" << std::endl;
            continue;
        }

        if (skipThread[threadIndex.value ()])
        {
            continue;
        }

        if (messagesIndexPerThread[threadIndex.value ()] >= expectedSequence.size ())
        {
            ++errorsDetected;
            std::cout << "ERROR: All messages for thread " << threadIndex.value () << " are already found!"
                      << std::endl;
            continue;
        }

        const Level expectedLevel = expectedSequence[messagesIndexPerThread[threadIndex.value ()]];
        const char *expectedMessage = LevelToMessage (expectedLevel);

        if (!line.ends_with (expectedMessage))
        {
            ++errorsDetected;
            std::cout << "ERROR on line " << lineNumber << ": Expected to end with \"" << expectedMessage
                      << "\". Line: \"" << line << "\"." << std::endl;

            skipThread[threadIndex.value ()] = true;
            continue;
        }

        ++messagesIndexPerThread[threadIndex.value ()];
    }

    return errorsDetected;
}

int main (int _argumentCount, const char **_arguments)
{
    if (_argumentCount != 2u)
    {
        std::cout << "ERROR: Expected one argument: path to logger executable!" << std::endl;
        return RESULT_ERROR;
    }

    std::cout << "Executing logger \"" << _arguments[ARGUMENT_LOGGER] << "\"..." << std::endl;
    const std::string command = _arguments[ARGUMENT_LOGGER] + std::string (" > ") + STDOUT_FILE + " 2> " + STDERR_FILE;
    system (command.c_str ());
    std::cout << "Execution finished." << std::endl;
    std::size_t errorsDetected = 0u;

    auto checkFile = [&errorsDetected] (const char *_file, Level _minimumLevel)
    {
        std::cout << "Checking file \"" << _file << "\"..." << std::endl;
        if (std::ifstream log {_file})
        {
            errorsDetected += CheckLog (log, _minimumLevel);
        }
        else
        {
            ++errorsDetected;
            std::cout << "ERROR: Unable to open log \"" << _file << "\"." << std::endl;
        }
    };

    checkFile (Test::GLOBAL_LOG_FILE_NAME, Test::GLOBAL_FILE_MINIMUM_LEVEL);
    checkFile (Test::CUSTOM_LOG_FILE_NAME, Test::CUSTOM_FILE_MINIMUM_LEVEL);
    checkFile (STDOUT_FILE, Test::STANDARD_OUT_MINIMUM_LEVEL);
    checkFile (STDERR_FILE, Test::STANDARD_ERROR_MINIMUM_LEVEL);

    std::cout << "Total errors count: " << errorsDetected << "." << std::endl;
    return errorsDetected == 0u ? RESULT_OK : RESULT_ERROR;
}