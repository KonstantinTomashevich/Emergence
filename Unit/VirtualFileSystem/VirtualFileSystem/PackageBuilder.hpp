#pragma once

#include <VirtualFileSystemApi.hpp>

#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

#include <VirtualFileSystem/Entry.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Helper class for building read-only packages. See Context documentation for more info.
class VirtualFileSystemApi PackageBuilder final
{
public:
    PackageBuilder () noexcept;

    PackageBuilder (const PackageBuilder &_other) = delete;

    PackageBuilder (PackageBuilder &&_other) noexcept;

    ~PackageBuilder () noexcept;

    /// \brief Attempts to begin package construction routine using given entry as resulting package output.
    bool Begin (const Context &_context, const Entry &_output) noexcept;

    /// \brief Attempts to add given file entry into package under given package-local path.
    bool Add (const Entry &_entry, const Container::Utf8String &_pathInPackage) noexcept;

    /// \brief Attempts to finalize package construction routine.
    bool End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PackageBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 21u);
};
} // namespace Emergence::VirtualFileSystem
