#include <Assert/Assert.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/Wrappers.hpp>

namespace Emergence::VirtualFileSystem
{
Context::Context () noexcept
{
    auto *holder = new (&data) Original::VirtualFileSystemHolder {};
    auto placeholder = holder->heap.GetAllocationGroup ().PlaceOnTop ();
    holder->virtualFileSystem =
        new (holder->heap.Acquire (sizeof (Original::VirtualFileSystem), alignof (Original::VirtualFileSystem)))
            Original::VirtualFileSystem {};
}

Context::Context (Context &&_context) noexcept
{
    new (&data)
        Original::VirtualFileSystemHolder {std::move (block_cast<Original::VirtualFileSystemHolder> (_context.data))};
    block_cast<Original::VirtualFileSystemHolder> (_context.data).virtualFileSystem = nullptr;
}

Context::~Context () noexcept
{
    auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    if (holder.virtualFileSystem)
    {
        holder.virtualFileSystem->~VirtualFileSystem ();
        holder.heap.Release (holder.virtualFileSystem, sizeof (Original::VirtualFileSystem));
    }

    holder.~VirtualFileSystemHolder ();
}

Entry Context::GetRoot () const noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);

    Original::EntryImplementationData entryData {holder.virtualFileSystem, {Original::ROOT_ID}};
    return Entry {array_cast (entryData)};
}

Entry Context::CreateFile (Entry _parent, const std::string_view &_fileName) noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);
    auto &parentData = block_cast<Original::EntryImplementationData> (_parent.data);
    EMERGENCE_ASSERT (parentData.owner == holder.virtualFileSystem);

    Original::EntryImplementationData entryData {holder.virtualFileSystem,
                                                 holder.virtualFileSystem->CreateFile (parentData.object, _fileName)};
    return Entry {array_cast (entryData)};
}

Entry Context::CreateDirectory (Entry _parent, const std::string_view &_directoryName) noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);
    auto &parentData = block_cast<Original::EntryImplementationData> (_parent.data);
    EMERGENCE_ASSERT (parentData.owner == holder.virtualFileSystem);

    Original::EntryImplementationData entryData {
        holder.virtualFileSystem, holder.virtualFileSystem->CreateDirectory (parentData.object, _directoryName)};
    return Entry {array_cast (entryData)};
}

Entry Context::MakeDirectories (const std::string_view &_absolutePath) noexcept
{
    return MakeDirectories (GetRoot (), _absolutePath);
}

Entry Context::MakeDirectories (Entry _parent, const std::string_view &_relativePath) noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);
    auto &parentData = block_cast<Original::EntryImplementationData> (_parent.data);
    EMERGENCE_ASSERT (parentData.owner == holder.virtualFileSystem);

    Original::EntryImplementationData entryData {
        holder.virtualFileSystem, holder.virtualFileSystem->MakeDirectories (parentData.object, _relativePath)};
    return Entry {array_cast (entryData)};
}

bool Context::Delete (const Entry &_entry, bool _recursive, bool _includingFileSystem) noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);
    const auto &entryData = block_cast<Original::EntryImplementationData> (_entry.data);
    EMERGENCE_ASSERT (entryData.owner == holder.virtualFileSystem);
    return holder.virtualFileSystem->Delete (entryData.object, _recursive, _includingFileSystem);
}

bool Context::Mount (const Entry &_at, const MountConfiguration &_configuration) noexcept
{
    const auto &holder = block_cast<Original::VirtualFileSystemHolder> (data);
    EMERGENCE_ASSERT (holder.virtualFileSystem);
    const auto &entryData = block_cast<Original::EntryImplementationData> (_at.data);
    EMERGENCE_ASSERT (entryData.owner == holder.virtualFileSystem);
    return holder.virtualFileSystem->Mount (entryData.object, _configuration);
}
} // namespace Emergence::VirtualFileSystem
