#include <cstring>
#include <fstream>

#include <FileSystem/Test/Utility.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::FileSystem::Test
{
void CheckFilesEquality (const Container::String &_expectedFilePath,
                         const Container::String &_resultFilePath,
                         float _maximumErrorRatio) noexcept
{
    std::ifstream expectedInput {_expectedFilePath.c_str (), std::ios::binary};
    REQUIRE (expectedInput);
    std::ifstream resultInput {_resultFilePath.c_str (), std::ios::binary};
    REQUIRE (resultInput);

    expectedInput.seekg (0u, std::ios::end);
    const std::streamsize expectedSize = expectedInput.tellg ();
    expectedInput.seekg (0u, std::ios::beg);

    resultInput.seekg (0u, std::ios::end);
    const std::streamsize resultSize = resultInput.tellg ();
    resultInput.seekg (0u, std::ios::beg);

    CHECK_EQUAL (expectedSize, resultSize);
    if (expectedSize != resultSize)
    {
        return;
    }

    constexpr auto BUFFER_SIZE = static_cast<std::streamsize> (16u * 1024u);
    std::streamsize read = 0u;
    std::array<std::uint8_t, BUFFER_SIZE> expectedBuffer;
    std::array<std::uint8_t, BUFFER_SIZE> resultBuffer;
    std::size_t errorCount = 0u;

    while (read < expectedSize)
    {
        const std::streamsize toRead = std::min (BUFFER_SIZE, expectedSize - read);
        read += toRead;

        REQUIRE (expectedInput.read (reinterpret_cast<char *> (expectedBuffer.data ()), toRead));
        REQUIRE (resultInput.read (reinterpret_cast<char *> (resultBuffer.data ()), toRead));

        for (std::size_t index = 0u; index < static_cast<std::size_t> (toRead); ++index)
        {
            if (expectedBuffer[index] != resultBuffer[index])
            {
                ++errorCount;
            }
        }
    }

    const float errorRatio = static_cast<float> (errorCount) / static_cast<float> (expectedSize);
    LOG ("File comparison error ratio: ", errorRatio, ".");
    CHECK ((errorRatio < _maximumErrorRatio));
}
} // namespace Emergence::FileSystem::Test
