#pragma once

#include <chrono>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Recording
{
using GroupUID = std::uint64_t;

constexpr GroupUID MISSING_GROUP_ID = std::numeric_limits<GroupUID>::max ();

enum class EventType : uint8_t
{
    DECLARE_GROUP = 0u,
    ALLOCATE,
    ACQUIRE,
    RELEASE,
    FREE,
    MARKER
};

struct Event
{
    EventType type;

    std::uint64_t timeNs;

    union
    {
        // Group declaration.
        struct
        {
            std::uint64_t reservedBytes;
            std::uint64_t acquiredBytes;
            GroupUID parent;
            UniqueString id;
        };

        // Memory usage events.
        struct
        {
            GroupUID group;
            std::uint64_t bytes;
        };

        // Marker.
        struct
        {
            GroupUID scope;
            UniqueString markerId;
        };
    };
};
} // namespace Emergence::Memory::Recording
