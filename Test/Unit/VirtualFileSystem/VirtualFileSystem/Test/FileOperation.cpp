#include <filesystem>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <VirtualFileSystem/Context.hpp>

namespace Emergence::VirtualFileSystem::Test
{
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

    CHECK (Entry {context, EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer", PATH_SEPARATOR, "Game")}
               .GetType () == EntryType::DIRECTORY);
}

TEST_CASE (CreateFileMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);
    CHECK (context.CreateFile (mounted, "test.txt").GetType () == EntryType::FILE);
    CHECK (std::filesystem::is_regular_file (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (CreateInvalidWeakFileLink)
{
    Context context;
    CHECK (context.CreateWeakFileLink (Entry {context, "test.txt"}, context.GetRoot (), "linked.txt").GetType () ==
           Emergence::VirtualFileSystem::EntryType::INVALID);
}

TEST_CASE (CreateValidWeakFileLink)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);
    CHECK (context.CreateFile (mounted, "test.txt").GetType () == EntryType::FILE);
    CHECK (context.CreateWeakFileLink (Entry {mounted, "test.txt"}, context.GetRoot (), "linked.txt").GetType () ==
           EntryType::FILE);
}

TEST_CASE (CreateValidWeakFileLinkOutsideOfVirtualHierarchy)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    REQUIRE (mounted.GetType () == EntryType::DIRECTORY);
    CHECK (context.CreateFile (mounted, "test.txt").GetType () == EntryType::FILE);
    CHECK (!context.CreateWeakFileLink (Entry {mounted, "test.txt"}, mounted, "linked.txt"));
}

TEST_CASE (ResolvePathWithDots)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry parent {context, EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "X", PATH_SEPARATOR, "Y")};
    CHECK (!parent);
    CHECK (!context.CreateFile (parent, "test.txt"));
}

TEST_CASE (CreateFileMountedMakeDirectories)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    CHECK (context.Delete (context.CreateFile (mounted, "test.txt"), false, true));
    CHECK (!std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteFileMounted)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    CHECK (context.Delete (context.CreateFile (mounted, "test.txt"), false, true));
    CHECK (!std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeleteWeakFileLinkKeepsFile)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    Entry file = context.CreateFile (mounted, "test.txt");
    Entry link = context.CreateWeakFileLink (file, context.GetRoot (), "linked.txt");

    CHECK (context.Delete (link, false, false));
    CHECK (file.GetType () == EntryType::FILE);
    CHECK (std::filesystem::exists (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")));
}

TEST_CASE (DeletingFileInvalidatesWeakFileLink)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    Entry file = context.CreateFile (mounted, "test.txt");
    Entry link = context.CreateWeakFileLink (file, context.GetRoot (), "linked.txt");

    CHECK (link.GetType () == EntryType::FILE);
    CHECK (context.Delete (file, false, true));
    CHECK (link.GetType () == EntryType::INVALID);
}

TEST_CASE (DeleteMountedDeleteReal)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

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

TEST_CASE (DeleteVirtualNonRecursive)
{
    Context context;
    Utf8String path {EMERGENCE_BUILD_STRING ("Resources", PATH_SEPARATOR, "Platformer", PATH_SEPARATOR, "Game")};
    context.MakeDirectories (path);
    CHECK (!context.Delete (Entry {context, "Resources"}, false, false));
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

TEST_CASE (LastWriteTime)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    Entry file = context.CreateFile (mounted, "test.txt");
    CHECK_EQUAL (std::filesystem::last_write_time (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")),
                 file.GetLastWriteTime ());
}

TEST_CASE (LastWriteTimeThroughLink)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    Entry file = context.CreateFile (mounted, "test.txt");
    Entry link = context.CreateWeakFileLink (file, context.GetRoot (), "linked.txt");

    CHECK_EQUAL (std::filesystem::last_write_time (EMERGENCE_BUILD_STRING (testDirectory, PATH_SEPARATOR, "test.txt")),
                 link.GetLastWriteTime ());
}

TEST_CASE (Naming)
{
    std::filesystem::remove_all (testDirectory);
    std::filesystem::create_directories (testDirectory);

    Context context;
    REQUIRE (context.Mount (context.GetRoot (),
                            MountConfiguration {MountSource::FILE_SYSTEM, testDirectory, testDirectory}));

    Entry mounted {context, testDirectory};
    Entry textFile = context.CreateFile (mounted, "test.txt");
    CHECK_EQUAL (textFile.GetName (), "test");
    CHECK_EQUAL (textFile.GetExtension (), "txt");
    CHECK_EQUAL (textFile.GetFullName (), "test.txt");

    Entry strangeFile = context.CreateFile (mounted, "test_no_extension");
    CHECK_EQUAL (strangeFile.GetName (), "test_no_extension");
    CHECK_EQUAL (strangeFile.GetExtension (), "");
    CHECK_EQUAL (strangeFile.GetFullName (), "test_no_extension");
}

END_SUITE
