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

    PackageBuilder (const PackageBuilder &_builder) = delete;

    PackageBuilder (PackageBuilder &&_builder) noexcept;

    ~PackageBuilder () noexcept;

    bool Begin (const Container::Utf8String &_outputFullPath) noexcept;

    bool Add (const Container::Utf8String &_pathInPackage, const Entry &_entry) noexcept;

    bool End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (PackageBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()
};
} // namespace Emergence::VirtualFileSystem
