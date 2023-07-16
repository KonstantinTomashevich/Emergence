#pragma once

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/Profiler/Original/AllocationGroup.hpp>

namespace Emergence::Memory::Profiler
{
class ImplementationUtils final
{
public:
    ImplementationUtils () = default;

    static Profiler::AllocationGroup ToServiceFormat (Original::AllocationGroup *_group) noexcept;
};
} // namespace Emergence::Memory::Profiler
