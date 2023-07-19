#pragma once

#include <API/Common/BlockCast.hpp>

#include <Galleon/CargoDeck.hpp>

namespace Emergence::Warehouse
{
struct RegistryData final
{
    Memory::Heap heap {Memory::Profiler::AllocationGroup (Memory::UniqueString {"Registry"})};
    Galleon::CargoDeck *deck = nullptr;
};
} // namespace Emergence::Warehouse
