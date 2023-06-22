#include <fstream>

#include <Testing/FileSystemTestUtility.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Testing
{
void CheckStreamEquality (std::istream &_expectedInput, std::istream &_resultInput, float _maximumErrorRatio) noexcept
{
    _expectedInput.seekg (0u, std::ios::end);
    const std::streamsize expectedSize = _expectedInput.tellg ();
    _expectedInput.seekg (0u, std::ios::beg);

    _resultInput.seekg (0u, std::ios::end);
    const std::streamsize resultSize = _resultInput.tellg ();
    _resultInput.seekg (0u, std::ios::beg);

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

        REQUIRE (_expectedInput.read (reinterpret_cast<char *> (expectedBuffer.data ()), toRead));
        REQUIRE (_resultInput.read (reinterpret_cast<char *> (resultBuffer.data ()), toRead));

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
} // namespace Emergence::Testing
