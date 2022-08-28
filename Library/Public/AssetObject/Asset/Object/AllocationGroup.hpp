#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Asset::Object
{
/// \return Root allocation group for all AssetObject library data.
Memory::Profiler::AllocationGroup GetRootAllocationGroup () noexcept;
} // namespace Emergence::Asset::Object
