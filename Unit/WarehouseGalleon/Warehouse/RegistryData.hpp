#pragma once

#include <Galleon/CargoDeck.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Warehouse
{
struct RegistryData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup (Memory::UniqueString {"Registry"})};
    Galleon::CargoDeck *deck = nullptr;
};
} // namespace Emergence::Warehouse
