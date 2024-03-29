#include <filesystem>
#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/Reader.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::VirtualFileSystem::Test
{
static const char *testDirectory = "ReadWriteTest";

void ComplicatedReadWriteTest (const Entry &_file)
{
    const Container::Utf8String fileText {
        "Hello, world! We avoid new lines here, because they are not really portable."};

    {
        Writer writer {_file};
        REQUIRE (writer);
        writer.OutputStream () << "WTFISIT" << fileText.substr (7u, 10u);
        writer.OutputStream ().seekp (0u, std::ios::beg);
        writer.OutputStream () << fileText.substr (0u, 7u);
        writer.OutputStream ().seekp (0u, std::ios::end);
        writer.OutputStream () << fileText.substr (17u);
    }

    Reader reader {_file};
    REQUIRE (reader);

    Container::StringBuilder textBuffer;
    int next;

    while ((next = reader.InputStream ().get ()) != EOF)
    {
        textBuffer.Append (static_cast<char> (next));
    }

    const Container::Utf8String resultText {textBuffer.Get ()};
    CHECK_EQUAL (resultText, fileText);
}
} // namespace Emergence::VirtualFileSystem::Test

using namespace Emergence::Container;
using namespace Emergence::VirtualFileSystem;
using namespace Emergence::VirtualFileSystem::Test;

BEGIN_SUITE (ReadWrite)

TEST_CASE (WriteReadText)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);
    const Utf8String fileText {"Hello, world! We avoid new lines here, because they are not really portable."};

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));

    {
        Writer writer {context.CreateFile (Entry {context, "Mounted"}, "test.txt")};
        REQUIRE (writer);
        writer.OutputStream () << fileText;
    }

    Reader reader {Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "test.txt")}};
    REQUIRE (reader);

    StringBuilder textBuffer;
    int next;

    while ((next = reader.InputStream ().get ()) != EOF)
    {
        textBuffer.Append (static_cast<char> (next));
    }

    const Utf8String resultText {textBuffer.Get ()};
    CHECK_EQUAL (resultText, fileText);
}

TEST_CASE (ComplicatedWriteReadText)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));
    Entry file = context.CreateFile (Entry {context, "Mounted"}, "test.txt");
    ComplicatedReadWriteTest (file);
}

TEST_CASE (WriteReadTextThroughLink)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);
    const Utf8String fileText {"Hello, world! We avoid new lines here, because they are not really portable."};

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));

    {
        Entry file = context.CreateFile (Entry {context, "Mounted"}, "test.txt");
        Entry link = context.CreateWeakFileLink (file, context.GetRoot (), "linked.txt");
        Writer writer {link};
        REQUIRE (writer);
        writer.OutputStream () << fileText;
    }

    Reader reader {Entry {context.GetRoot (), "linked.txt"}};
    REQUIRE (reader);

    StringBuilder textBuffer;
    int next;

    while ((next = reader.InputStream ().get ()) != EOF)
    {
        textBuffer.Append (static_cast<char> (next));
    }

    const Utf8String resultText {textBuffer.Get ()};
    CHECK_EQUAL (resultText, fileText);
}

TEST_CASE (WriteReadBinary)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));
    const Vector<std::uint8_t> fileContent {12u, 127u, 13u, 177u, 48u, 99u, 188u, 11u};

    {
        Writer writer {context.CreateFile (Entry {context, "Mounted"}, "test.bin")};
        REQUIRE (writer);
        writer.OutputStream ().write (reinterpret_cast<const char *> (fileContent.data ()),
                                      static_cast<std::streamsize> (fileContent.size ()));
    }

    Reader reader {Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "test.bin")}};
    REQUIRE (reader);

    Vector<std::uint8_t> readContent;
    int next;

    while ((next = reader.InputStream ().get ()) != EOF)
    {
        readContent.emplace_back (static_cast<std::uint8_t> (next));
    }

    CHECK_EQUAL (readContent, fileContent);
}

TEST_CASE (WriteReadVirtual)
{
    Context context;
    constexpr std::uint64_t TEST_CONTENT_SIZE = 1048576u;
    Vector<std::uint8_t> fileContent;
    fileContent.reserve (TEST_CONTENT_SIZE);

    for (std::uint64_t index = 0u; index < TEST_CONTENT_SIZE; ++index)
    {
        fileContent.emplace_back (static_cast<std::uint8_t> (index));
    }

    {
        Writer writer {context.CreateFile (context.GetRoot (), "test.bin")};
        REQUIRE (writer);
        writer.OutputStream ().write (reinterpret_cast<const char *> (fileContent.data ()),
                                      static_cast<std::streamsize> (fileContent.size ()));
    }

    Reader reader {Entry {context.GetRoot (), "test.bin"}};
    REQUIRE (reader);

    Vector<std::uint8_t> readContent;
    int next;

    while ((next = reader.InputStream ().get ()) != EOF)
    {
        readContent.emplace_back (static_cast<std::uint8_t> (next));
    }

    CHECK_EQUAL (readContent, fileContent);
}

TEST_CASE (ComplicatedWriteReadVirtualText)
{
    Context context;
    Entry file = context.CreateFile (context.GetRoot (), "test.bin");
    ComplicatedReadWriteTest (file);
}

END_SUITE
