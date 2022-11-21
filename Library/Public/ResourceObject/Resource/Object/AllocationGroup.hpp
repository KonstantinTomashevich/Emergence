#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Resource::Object
{
/// \return Root allocation group for all ResourceObject library data.
Memory::Profiler::AllocationGroup GetRootAllocationGroup () noexcept;
} // namespace Emergence::Resource::Object
