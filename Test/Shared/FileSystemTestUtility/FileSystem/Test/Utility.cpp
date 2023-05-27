#include <fstream>

#include <FileSystem/Test/Utility.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::FileSystem::Test
{
void ExpectFilesEqual (const Container::String &_expectedFilePath, const Container::String &_resultFilePath) noexcept
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

    while (read < expectedSize)
    {
        const std::streamsize toRead = std::min (BUFFER_SIZE, expectedSize - read);
        read += toRead;

        REQUIRE (expectedInput.read (reinterpret_cast<char *> (expectedBuffer.data ()), toRead));
        REQUIRE (resultInput.read (reinterpret_cast<char *> (resultBuffer.data ()), toRead));
        CHECK (memcmp (expectedBuffer.data (), resultBuffer.data (), toRead) == 0);
    }
}
} // namespace Emergence::FileSystem::Test
