#include <Testing/SetupMain.hpp>

#include <VirtualFileSystem/Test/FileOperation.hpp>
#include <VirtualFileSystem/Test/Mount.hpp>
#include <VirtualFileSystem/Test/PackageFile.hpp>
#include <VirtualFileSystem/Test/ReadWrite.hpp>

// Use "include marker", so linker will not omit translation units with autoregistered tests.
static const bool FILE_OPERATION_TEST_INCLUDE_MARKER =
    Emergence::VirtualFileSystem::Test::FileOperationTestIncludeMarker ();
static const bool MOUNT_TEST_INCLUDE_MARKER = Emergence::VirtualFileSystem::Test::MountTestIncludeMarker ();
static const bool PACKAGE_FILE_TEST_INCLUDE_MARKER =
    Emergence::VirtualFileSystem::Test::PackageFileTestIncludeMarker ();
static const bool READ_WRITE_TEST_INCLUDE_MARKER = Emergence::VirtualFileSystem::Test::ReadWriteTestIncludeMarker ();
