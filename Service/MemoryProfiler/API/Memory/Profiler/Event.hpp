#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

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
    const char *groupId = nullptr;
    EventType type = EventType::ALLOCATE;
    size_t bytesCount = 0u;
    std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

class EventIterator final
{
public:
    EMERGENCE_FORWARD_ITERATOR_OPERATIONS (EventIterator, const Event *);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    explicit EventIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};
} // namespace Emergence::Memory::Profiler
