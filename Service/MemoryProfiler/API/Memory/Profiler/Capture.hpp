#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
enum class EventType
{
    ALLOCATE = 0,
    ACQUIRE,
    RELEASE,
    FREE
};

struct Event final
{
    AllocationGroup allocationGroup;

    EventType type = EventType::ALLOCATE;

    size_t bytes = 0u;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

class Capture
{
public:
    Capture () noexcept;

    /// Copying captures seems counter-intuitive.
    Capture (const Capture &_other) = delete;

    Capture (Capture &&_other) noexcept;

    ~Capture () = default;

    const Event *NextEvent () noexcept;

    /// Assigning captures seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Capture);

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Memory::Profiler
