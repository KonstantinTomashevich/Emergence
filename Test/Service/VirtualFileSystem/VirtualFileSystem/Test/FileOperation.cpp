#include <filesystem>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/Test/FileOperation.hpp>

namespace Emergence::VirtualFileSystem::Test
{
bool FileOperationTestIncludeMarker () noexcept
{
    return true;
}

static const char *testDirectory = "FileOperationTest";
} // namespace Emergence::VirtualFileSystem::Test

using namespace Emergence::Container;
using namespace Emergence::VirtualFileSystem;
using namespace Emergence::VirtualFileSystem::Test;

BEGIN_SUITE (FileOperation)

TEST_CASE (CreateFileUnmapped)
{
    Context context;
    CHECK (!context.CreateFile (context.GetRoot (), "test.txt"));
}

TEST_CASE (CreateVirtualDirectory)
{
    Context context;
    CHECK (context.CreateDirectory (context.GetRoot (), "Resources").GetType () == EntryType::DIRECTORY);
}

TEST_CASE (MakeDirectoriesVirtual)
{
    Context context;
    CHECK (context
               .MakeDirectories (
                   EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer", PATH_SEPARATOR, "Game"))
               .GetType () == EntryType::DIRECTORY);
}

TEST_CASE (MakeDirectoriesVirtualWithDots)
{
    Context context;
    CHECK (context
               .MakeDirectories (EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer", PATH_SEPARATOR,
                                                         "Game", PATH_SEPARATOR, "..", PATH_SEPARATOR, ".",
                                                         PATH_SEPARATOR, "Menu"))
               .GetType () == EntryType::DIRECTORY);
}

TEST_CASE (CreateFileMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);
    CHECK (context.CreateFile (mounted, "test.txt").GetType () == EntryType::FILE);
    CHECK (std::filesystem::is_regular_file (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (ResolvePathWithDots)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    context.CreateFile (mounted, "test.txt");

    CHECK (Entry {context, EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "..", PATH_SEPARATOR, testDirectory,
                                                   PATH_SEPARATOR, ".", PATH_SEPARATOR, "test.txt")}
               .GetType () == EntryType::FILE);
}

TEST_CASE (CreateDirectoryMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);
    CHECK (context.CreateDirectory (mounted, "test").GetType () == EntryType::DIRECTORY);
    CHECK (std::filesystem::is_directory (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test")));
}

TEST_CASE (MakeDirectoriesMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);

    CHECK (context.MakeDirectories (mounted, EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer"))
               .GetType () == EntryType::DIRECTORY);

    CHECK (std::filesystem::is_directory (
        EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "Resources", PATH_SEPARATOR, "Platformer")));
}

TEST_CASE (MakeDirectoriesMixed)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);

    CHECK (context
               .MakeDirectories (
                   EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "Resources", PATH_SEPARATOR, "Platformer"))
               .GetType () == EntryType::DIRECTORY);

    CHECK (std::filesystem::is_directory (
        EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "Resources", PATH_SEPARATOR, "Platformer")));
}

TEST_CASE (CreateFileMountedNoDirectory)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry parent {context, EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "X", PATH_SEPARATOR, "Y")};
    CHECK (!parent);
    CHECK (!context.CreateFile (parent, "test.txt"));
}

TEST_CASE (CreateFileMountedMakeDirectories)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry parent {
        context.MakeDirectories (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "X", PATH_SEPARATOR, "Y"))};
    CHECK (parent.GetType () == EntryType::DIRECTORY);

    CHECK (context.CreateFile (parent, "test.txt").GetType () == EntryType::FILE);
    CHECK (std::filesystem::is_regular_file (
        EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "X", PATH_SEPARATOR, "Y", PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteUnexistentFile)
{
    Context context;
    CHECK (!context.Delete (Entry {context, "Test/temp.txt"}, false, true));
}

TEST_CASE (DeleteFileMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    CHECK (context.Delete (context.CreateFile (mounted, "test.txt"), false, true));
    CHECK (!std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteMountedKeepReal)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    context.CreateFile (mounted, "test.txt");

    CHECK (context.Delete (mounted, true, false));
    CHECK (std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteMountedDeleteReal)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (
        context.Mount (context.GetRoot (), MountConfiguration {MountSource::FILESYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    context.CreateFile (mounted, "test.txt");

    CHECK (context.Delete (mounted, true, true));
    CHECK (!std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteVirtualRecursive)
{
    Context context;
    Utf8String path {EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer", PATH_SEPARATOR, "Game")};
    context.MakeDirectories (path);

    CHECK (context.Delete (Entry {context, "Resources"}, true, false));
    CHECK (!Entry {context, path});
}

TEST_CASE (IterateVirtual)
{
    Context context;
    context.CreateDirectory (context.GetRoot (), "Resources");
    context.CreateDirectory (context.GetRoot (), "Logs");
    context.CreateDirectory (context.GetRoot (), "ThirdParty");

    auto cursor = context.GetRoot ().ReadChildren ();
    Vector<Utf8String> children;

    while (Entry entry = *cursor)
    {
        CHECK (entry.GetType () == EntryType::DIRECTORY);
        children.emplace_back (entry.GetFullPath ());
        ++cursor;
    }

    CHECK (children.size () == 3u);
    CHECK (std::find (children.begin (), children.end (), "~/Resources") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/Logs") != children.end ());
    CHECK (std::find (children.begin (), children.end (), "~/ThirdParty") != children.end ());
}

END_SUITE
