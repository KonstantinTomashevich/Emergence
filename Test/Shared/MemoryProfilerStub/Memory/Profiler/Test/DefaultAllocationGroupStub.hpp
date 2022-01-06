#pragma once

#include <Memory/Heap.hpp>

namespace Emergence::Memory
{
// We do not need to profile memory in tests, therefore we are adding this stub for convenience.
template <typename T>
struct DefaultAllocationGroup
{
    static Profiler::AllocationGroup Get () noexcept
    {
        static Profiler::AllocationGroup group {UniqueString {"TestStub"}};
        return group;
    }
};
} // namespace Emergence::Memory
