#pragma once

#include <VirtualFileSystem/Original/Core.hpp>

namespace Emergence::VirtualFileSystem::Original
{
struct VirtualFileSystemHolder final
{
    VirtualFileSystem *virtualFileSystem = nullptr;
    Memory::Heap heap {Memory::Profiler::AllocationGroup {Memory::UniqueString {"VirtualFileSystem"}}};
};

struct EntryImplementationData final
{
    VirtualFileSystem *owner = nullptr;
    Object object;
};

struct CursorImplementationData final
{
    VirtualFileSystem *owner = nullptr;
    VirtualFileSystem::Cursor cursor;
};
} // namespace Emergence::VirtualFileSystem::Original
