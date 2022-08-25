#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Asset::Object
{
Memory::Profiler::AllocationGroup GetRootAllocationGroup () noexcept;
} // namespace Emergence::Asset::Object
