#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Memory::Profiler
{
void AddMarker (UniqueString /*unused*/, const AllocationGroup & /*unused*/) noexcept
{
}

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator++ () noexcept
{
    return *this;
}

CapturedAllocationGroup::Iterator CapturedAllocationGroup::Iterator::operator++ (int) noexcept
{
    return Iterator {{}};
}

CapturedAllocationGroup CapturedAllocationGroup::Iterator::operator* () const noexcept
{
    return CapturedAllocationGroup {nullptr};
}

CapturedAllocationGroup::Iterator::Iterator (const CapturedAllocationGroup::Iterator &_other) noexcept = default;

CapturedAllocationGroup::Iterator::Iterator (CapturedAllocationGroup::Iterator &&_other) noexcept = default;

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator= (
    const CapturedAllocationGroup::Iterator &_other) noexcept = default;

CapturedAllocationGroup::Iterator &CapturedAllocationGroup::Iterator::operator= (
    CapturedAllocationGroup::Iterator &&_other) noexcept = default;

CapturedAllocationGroup::Iterator::~Iterator () noexcept = default;

bool CapturedAllocationGroup::Iterator::operator== (const CapturedAllocationGroup::Iterator & /*unused*/) const noexcept
{
    return true;
}

bool CapturedAllocationGroup::Iterator::operator!= (const CapturedAllocationGroup::Iterator & /*unused*/) const noexcept
{
    return false;
}

CapturedAllocationGroup::Iterator::Iterator (const std::array<std::uint8_t, DATA_MAX_SIZE> & /*unused*/) noexcept
{
    // Suppress unused class field warning.
    [[maybe_unused]] auto *stub = &data;
}

CapturedAllocationGroup::CapturedAllocationGroup (const CapturedAllocationGroup &_other) noexcept = default;

CapturedAllocationGroup::CapturedAllocationGroup (CapturedAllocationGroup &&_other) noexcept = default;

CapturedAllocationGroup::~CapturedAllocationGroup () noexcept = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
CapturedAllocationGroup::Iterator CapturedAllocationGroup::BeginChildren () const noexcept
{
    return Iterator {{}};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
CapturedAllocationGroup::Iterator CapturedAllocationGroup::EndChildren () const noexcept
{
    return Iterator {{}};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
UniqueString CapturedAllocationGroup::GetId () const noexcept
{
    return UniqueString {};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
std::size_t CapturedAllocationGroup::GetAcquired () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
std::size_t CapturedAllocationGroup::GetReserved () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
std::size_t CapturedAllocationGroup::GetTotal () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
AllocationGroup CapturedAllocationGroup::GetSource () const noexcept
{
    return AllocationGroup {};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
std::uint64_t CapturedAllocationGroup::GetCaptureTimeNs () const noexcept
{
    return 0u;
}

CapturedAllocationGroup::CapturedAllocationGroup (void *_handle) noexcept
    : handle (_handle)
{
    // Suppress unused class field warning.
    [[maybe_unused]] auto *stub = &handle;
}

EventObserver::EventObserver (EventObserver &&_other) noexcept = default;

EventObserver::~EventObserver () noexcept = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
const Event *EventObserver::NextEvent () noexcept
{
    return nullptr;
}

EventObserver::EventObserver (std::array<std::uint8_t, DATA_MAX_SIZE> & /*unused*/) noexcept
{
    // Suppress unused class field warning.
    [[maybe_unused]] auto *stub = &data;
}

std::pair<CapturedAllocationGroup, EventObserver> Capture::Start () noexcept
{
    static std::array<std::uint8_t, EventObserver::DATA_MAX_SIZE> stub;
    return {CapturedAllocationGroup {nullptr}, EventObserver {stub}};
}
} // namespace Emergence::Memory::Profiler
