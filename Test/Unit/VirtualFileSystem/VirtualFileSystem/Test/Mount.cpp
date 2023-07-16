#include <filesystem>
#include <fstream>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Context.hpp>

namespace Emergence::VirtualFileSystem::Test
{
static const char *testDirectory = "MountTest";
} // namespace Emergence::VirtualFileSystem::Test

using namespace Emergence::Container;
using namespace Emergence::VirtualFileSystem;
using namespace Emergence::VirtualFileSystem::Test;

BEGIN_SUITE (Mount)

TEST_CASE (MountOneDirectory)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);

    {
        std::ofstream firstFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "first.txt")};
        std::ofstream secondFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "second.txt")};
        std::ofstream thirdFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "third.txt")};
    }

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "first.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "second.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "third.txt")}.GetType () ==
           EntryType::FILE);
}

TEST_CASE (MountTwoDirectories)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String firstPath = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (firstPath);

    {
        std::ofstream firstFile {EMERGENCE_BUILD_STRING (firstPath, PATH_SEPARATOR, "first.txt")};
        std::ofstream secondFile {EMERGENCE_BUILD_STRING (firstPath, PATH_SEPARATOR, "second.txt")};
    }

    const Utf8String secondPath = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "Second");
    std::filesystem::create_directories (secondPath);

    {
        std::ofstream thirdFile {EMERGENCE_BUILD_STRING (secondPath, PATH_SEPARATOR, "third.txt")};
    }

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, firstPath, "Mounted1"}));
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, secondPath, "Mounted2"}));

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted1", PATH_SEPARATOR, "first.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted1", PATH_SEPARATOR, "second.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted2", PATH_SEPARATOR, "third.txt")}.GetType () ==
           EntryType::FILE);
}

TEST_CASE (MountOneDirectoryWithNesting)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);

    {
        std::ofstream firstFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "first.txt")};
        std::ofstream secondFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "second.txt")};

        std::filesystem::create_directories (EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "Nested"));
        std::ofstream thirdFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "Nested", PATH_SEPARATOR, "third.txt")};
    }

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "first.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (), EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "second.txt")}.GetType () ==
           EntryType::FILE);

    CHECK (Entry {context.GetRoot (),
                  EMERGENCE_BUILD_STRING ("Mounted", PATH_SEPARATOR, "Nested", PATH_SEPARATOR, "third.txt")}
               .GetType () == EntryType::FILE);
}

TEST_CASE (IterateMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    const Utf8String path = EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "First");
    std::filesystem::create_directories (path);

    {
        std::ofstream firstFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "first.txt")};
        std::ofstream secondFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "second.txt")};

        std::filesystem::create_directories (EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "Nested"));
        std::ofstream thirdFile {EMERGENCE_BUILD_STRING (path, PATH_SEPARATOR, "Nested", PATH_SEPARATOR, "third.txt")};
    }

    Context context;
    REQUIRE (context.Mount (context.GetRoot (), {MountSource::FILE_SYSTEM, path, "Mounted"}));

    auto cursor = Entry {context, "Mounted"}.ReadChildren ();
    Emergence::Container::Vector<Emergence::Container::Utf8String> children;

    while (Entry entry = *cursor)
    {
        children.emplace_back (entry.GetFullPath ());
        ++cursor;
    }

    CHECK (children.size () == 3u);
    CHECK (std::find (children.begin (), children.end (), "~/Mounted/first.txt") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/Mounted/second.txt") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/Mounted/Nested") != children.end ());
}

END_SUITE
