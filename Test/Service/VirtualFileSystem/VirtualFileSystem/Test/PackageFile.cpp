#include <filesystem>
#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/PackageBuilder.hpp>
#include <VirtualFileSystem/Reader.hpp>
#include <VirtualFileSystem/Test/PackageFile.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::VirtualFileSystem::Test
{
bool PackageFileTestIncludeMarker () noexcept
{
    return true;
}

static const char *testDirectory = "PackageFileTest";
} // namespace Emergence::VirtualFileSystem::Test

using namespace Emergence::Container;
using namespace Emergence::VirtualFileSystem;
using namespace Emergence::VirtualFileSystem::Test;

BEGIN_SUITE (PackageFile)

TEST_CASE (PackageFileWithNesting)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String packageSourcePath = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "PackageSource");
    const Utf8String packageOutputPath = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "PackageOutput");
    std::filesystem::create_directories (packageSourcePath);
    std::filesystem::create_directories (packageOutputPath);

    const Utf8String firstText = "First one!";
    const Utf8String secondText = "Second one!";
    const Utf8String thirdText = "Third one!";

    {
        std::ofstream firstFile {EMERGENCE_BUILD_STRING (packageSourcePath, PATH_SEPARATOR, "first.txt")};
        firstFile << firstText;

        std::ofstream secondFile {EMERGENCE_BUILD_STRING (packageSourcePath, PATH_SEPARATOR, "second.txt")};
        secondFile << secondText;

        std::ofstream thirdFile {EMERGENCE_BUILD_STRING (packageSourcePath, PATH_SEPARATOR, "third.txt")};
        thirdFile << thirdText;
    }

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, packageSourcePath, "Source"}));
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, packageOutputPath, "Output"}));

    {
        PackageBuilder builder;
        REQUIRE (builder.Begin (context, context.CreateFile (Entry {context, "Output"}, "Package.bin")));

        REQUIRE (
            builder.Add (Entry {context, EMERGENCE_BUILD_STRING ("Source", PATH_SEPARATOR, "first.txt")}, "1.txt"));

        REQUIRE (
            builder.Add (Entry {context, EMERGENCE_BUILD_STRING ("Source", PATH_SEPARATOR, "second.txt")}, "2.txt"));

        REQUIRE (builder.Add (Entry {context, EMERGENCE_BUILD_STRING ("Source", PATH_SEPARATOR, "third.txt")},
                              EMERGENCE_BUILD_STRING ("Nested", PATH_SEPARATOR, "3.txt")));

        REQUIRE (builder.End ());
    }

    REQUIRE (context.Mount (
        context.GetRoot (),
        {MountSource::PACKAGE, EMERGENCE_BUILD_STRING (packageOutputPath, PATH_SEPARATOR, "Package.bin"), "Package"}));

    // Test that entries exist.

    Entry packagedFirst {context, EMERGENCE_BUILD_STRING ("Package", PATH_SEPARATOR, "1.txt")};
    Entry packagedSecond {context, EMERGENCE_BUILD_STRING ("Package", PATH_SEPARATOR, "2.txt")};
    Entry packagedThird {context,
                         EMERGENCE_BUILD_STRING ("Package", PATH_SEPARATOR, "Nested", PATH_SEPARATOR, "3.txt")};

    CHECK (packagedFirst);
    CHECK (packagedSecond);
    CHECK (packagedThird);

    // Test that entries readable.

    {
        auto testReadFile = [] (const Entry &_entry, const Utf8String &_expected)
        {
            Reader reader {_entry, OpenMode::BINARY};
            REQUIRE (reader);

            StringBuilder textBuffer;
            int next;

            while ((next = reader.InputStream ().get ()) != EOF)
            {
                textBuffer.Append (static_cast<char> (next));
            }

            const Utf8String resultText {textBuffer.Get ()};
            CHECK_EQUAL (resultText, _expected);
        };

        testReadFile (packagedFirst, firstText);
        testReadFile (packagedSecond, secondText);
        testReadFile (packagedThird, thirdText);
    }

    // Test that entries not writable.

    {
        Writer firstWriter {packagedFirst, OpenMode::BINARY};
        CHECK (!firstWriter);

        Writer secondWriter {packagedSecond, OpenMode::BINARY};
        CHECK (!secondWriter);

        Writer thirdWriter {packagedThird, OpenMode::BINARY};
        CHECK (!thirdWriter);
    }

    // Test entry iteration.

    auto cursor = Entry {context, "Package"}.ReadChildren ();
    Emergence::Container::Vector<Emergence::Container::Utf8String> children;

    while (Entry entry = *cursor)
    {
        children.emplace_back (entry.GetFullPath ());
        ++cursor;
    }

    CHECK (children.size () == 3u);
    CHECK (std::find (children.begin (), children.end (), "~/Package/1.txt") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/Package/2.txt") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/Package/Nested") != children.end ());
}

END_SUITE
