#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory::Test
{
/// \brief We need to get unique groups for every allocator in order to check their memory usage through profiler.
Profiler::AllocationGroup GetUniqueAllocationGroup () noexcept;
} // namespace Emergence::Memory::Test
