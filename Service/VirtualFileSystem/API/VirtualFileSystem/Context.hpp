#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::VirtualFileSystem
{
class Context final
{
public:
    Context () noexcept;

    Context (const Context &_context) = delete;

    Context (Context &&_context) noexcept;

    ~Context () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Context);

    Entry GetRoot () noexcept;

    Entry Create (const Container::Utf8String &_fullPath, EntryType _type) noexcept;

    bool Mount (const Entry &_at, const MountConfiguration &_configuration) noexcept;

    bool Delete (const Entry &_entry) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()
};
} // namespace Emergence::VirtualFileSystem
