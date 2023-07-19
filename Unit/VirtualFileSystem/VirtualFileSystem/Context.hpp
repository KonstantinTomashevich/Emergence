#pragma once

#include <VirtualFileSystemApi.hpp>

#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Symbol used to separate path entries across the virtual file system.
constexpr const char PATH_SEPARATOR = '/';

/// \brief Core object that manages state of the whole virtual file system.
///
/// \par Mounting
/// \parblock
/// Newly created virtual file system instance is pure and empty: it knows nothing about program environment like
/// current working directory, path variables, etc. User is fully responsible to declare what virtual file system
/// observes by mounting the required paths and read-only packages into virtual file system.
/// \endparblock
///
/// \par Thread safety
/// \parblock
/// By default, all methods of all virtual file system classes are thread safe.
/// The only exception is non-constant methods of this class: when using them, user must guarantee that no other
/// virtual file system operation is going on. The reason is that these operations modify virtual file system entries
/// tree and therefore affect how the whole system works. It was decided that it is better to allow users to construct
/// their own lock-free environment for virtual file system calls instead of using shared locks under the hood.
/// \endparblock
///
/// \par Read-only packages
/// \parblock
/// Reading lots of small files from the drive is often quite slow. Typical solution for that is read-only packages:
/// required read-only files are packed into one big file that consists of header section and data section. Header
/// sections contains file paths, offsets and sizes in data section, while data section contains actual file data.
/// Virtual file system provides tools for building such files and is able to mount them like directories.
/// \endparblock
///
/// \par Multiple virtual file systems
/// \parblock
/// You can safely create multiple virtual file systems under one process by creating multiple context instances.
/// \endparblock
class VirtualFileSystemApi Context final
{
public:
    /// \brief Constructs new empty instance of virtual file system.
    Context () noexcept;

    Context (const Context &_context) = delete;

    Context (Context &&_context) noexcept;

    /// \invariant All other objects that use this virtual file system instance must be destroyed earlier.
    ~Context () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Context);

    /// \brief Root directory of virtual file system.
    [[nodiscard]] Entry GetRoot () const noexcept;

    /// \brief Creates empty file with given name that is child of given virtual file system entry.
    /// \invariant Given parent entry must be mappable to real file system path, we do not support fully virtual files.
    Entry CreateFile (const Entry &_parent, const std::string_view &_fileName) noexcept;

    /// \brief Creates new directory with given name that is child of given virtual file system entry.
    /// \details If given parent entry is mappable to real file system path, then new directory will be created
    ///          in real file system too. Otherwise, new directory will exists only in virtual file system.
    Entry CreateDirectory (const Entry &_parent, const std::string_view &_directoryName) noexcept;

    /// \brief Creates new weak file link with given name that is child of given
    ///        virtual file system entry and points to given target entry.
    /// \invariant Given parent entry must be a virtual directory: real file system driven links are not supported yet.
    Entry CreateWeakFileLink (const Entry &_target, const Entry &_parent, const std::string_view &_linkName) noexcept;

    /// \brief Ensure that given absolute path is valid by creating all missing directories along the way.
    Entry MakeDirectories (const std::string_view &_absolutePath) noexcept;

    /// \brief Ensure that given path, relative to given parent, is
    ///        valid by creating all missing directories along the way.
    Entry MakeDirectories (const Entry &_parent, const std::string_view &_relativePath) noexcept;

    /// \brief Attempts to delete given virtual file system entry.
    /// \param _entry Entry to delete.
    /// \param _recursive If true, all children of this entry will be deleted too.
    ///                   If not and entry has children, it won't be deleted at all.
    /// \param _includingFileSystem If true, real file system files and directories will be deleted as well.
    ///                             Otherwise, only virtual file system entries will be deleted. This behaviour
    ///                             is useful for unmounting: we can delete virtual mount points while keeping
    ///                             files of real file system intact.
    /// \warning Keep in mind that weak file link deletion does not result in file deletion!
    bool Delete (const Entry &_entry, bool _recursive, bool _includingFileSystem) noexcept;

    /// \brief Attempts to mount given configuration as child of given entry that must be virtual directory.
    bool Mount (const Entry &_at, const MountConfiguration &_configuration) noexcept;

private:
    friend class Entry;
    friend class PackageBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 2u);
};
} // namespace Emergence::VirtualFileSystem
