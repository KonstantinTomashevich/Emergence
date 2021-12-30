#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

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
    UniqueString registryId;

    EventType type = EventType::ALLOCATE;

    size_t bytes = 0u;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;

    Event *next = nullptr;

    size_t startOfCapture = 0u;
};

class CapturedRegistry final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, CapturedRegistry);

    private:
        /// Registry constructs iterators.
        friend class CapturedRegistry;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    Iterator BeginChildren () const noexcept;

    Iterator EndChildren () const noexcept;

    UniqueString GetId () const noexcept;

    size_t GetAcquired () const noexcept;

    size_t GetReserved () const noexcept;

    size_t GetTotal () const noexcept;

private:
    friend class Capture;

    CapturedRegistry (const void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};

class Capture
{
public:
    Capture () noexcept;

    /// Copying captures seems counter-intuitive.
    Capture (const Capture &_other) = delete;

    Capture (Capture &&_other) noexcept;

    ~Capture () = default;

    const Event &Begin () const noexcept;

    const Event &Current () const noexcept;

    const Event *GoToNext () noexcept;

    const Event *GoToPrevious () noexcept;

    const CapturedRegistry::Iterator BeginInitial () const noexcept;

    const CapturedRegistry::Iterator EndInitial () const noexcept;

    const CapturedRegistry::Iterator BeginCurrent () const noexcept;

    const CapturedRegistry::Iterator EndCurrent () const noexcept;

    /// Assigning captures seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Capture);

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Memory::Profiler
