#pragma once

#include <Memory/Heap.hpp>

namespace Emergence::Memory
{
// We already have default mock for non-MSVC builds as ad-hoc.
#if defined(_MSVC_STL_VERSION)
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
#endif
} // namespace Emergence::Memory
