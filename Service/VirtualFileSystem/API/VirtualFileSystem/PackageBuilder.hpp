#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

#include <VirtualFileSystem/Entry.hpp>

namespace Emergence::VirtualFileSystem
{
class PackageBuilder final
{
public:
    PackageBuilder () noexcept;

    PackageBuilder (const PackageBuilder &_other) = delete;

    PackageBuilder (PackageBuilder &&_other) noexcept;

    ~PackageBuilder () noexcept;

    bool Begin (const Context &_context, const Entry &_output) noexcept;

    bool Add (const Entry &_entry, const Container::Utf8String &_pathInPackage) noexcept;

    bool End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PackageBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 21u);
};
} // namespace Emergence::VirtualFileSystem
