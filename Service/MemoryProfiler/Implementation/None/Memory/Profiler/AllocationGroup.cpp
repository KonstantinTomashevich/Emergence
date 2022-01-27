#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory::Profiler
{
AllocationGroup::Iterator &AllocationGroup::Iterator::operator++ () noexcept
{
    return *this;
}

AllocationGroup::Iterator AllocationGroup::Iterator::operator++ (int) noexcept
{
    return Iterator {nullptr};
}

AllocationGroup AllocationGroup::Iterator::operator* () const noexcept
{
    return {};
}

AllocationGroup::Iterator::Iterator (const AllocationGroup::Iterator &_other) noexcept = default;

AllocationGroup::Iterator::Iterator (AllocationGroup::Iterator &&_other) noexcept = default;

AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (const AllocationGroup::Iterator &_other) noexcept =
    default;

AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (AllocationGroup::Iterator &&_other) noexcept = default;

AllocationGroup::Iterator::~Iterator () noexcept = default;

bool AllocationGroup::Iterator::operator== (const AllocationGroup::Iterator & /*unused*/) const noexcept
{
    return true;
}

bool AllocationGroup::Iterator::operator!= (const AllocationGroup::Iterator & /*unused*/) const noexcept
{
    return false;
}

AllocationGroup::Iterator::Iterator (const std::array<uint8_t, DATA_MAX_SIZE> * /*unused*/) noexcept
{
    // Suppress unused class field warning.
    [[maybe_unused]] auto *stub = &data;
}

AllocationGroup::PlacedOnStack::~PlacedOnStack () = default;

AllocationGroup::PlacedOnStack::PlacedOnStack (void * /*unused*/) noexcept
{
}

AllocationGroup AllocationGroup::Root () noexcept
{
    return {};
}

AllocationGroup AllocationGroup::Top () noexcept
{
    return {};
}

AllocationGroup::AllocationGroup () noexcept : handle (nullptr)
{
}

AllocationGroup::AllocationGroup (UniqueString /*unused*/) noexcept : handle (nullptr)
{
}

AllocationGroup::AllocationGroup (const AllocationGroup & /*unused*/, UniqueString /*unused*/) noexcept
    : handle (nullptr)
{
}

AllocationGroup::AllocationGroup (const AllocationGroup &_other) noexcept = default;

AllocationGroup::AllocationGroup (AllocationGroup &&_other) noexcept = default;

AllocationGroup::~AllocationGroup () noexcept = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
AllocationGroup::PlacedOnStack AllocationGroup::PlaceOnTop () const noexcept
{
    return PlacedOnStack {nullptr};
}

void AllocationGroup::Allocate (size_t /*unused*/) noexcept
{
}

void AllocationGroup::Acquire (size_t /*unused*/) noexcept
{
}

void AllocationGroup::Release (size_t /*unused*/) noexcept
{
}

void AllocationGroup::Free (size_t /*unused*/) noexcept
{
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
AllocationGroup AllocationGroup::Parent () const noexcept
{
    return {};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
AllocationGroup::Iterator AllocationGroup::BeginChildren () const noexcept
{
    return Iterator {nullptr};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
AllocationGroup::Iterator AllocationGroup::EndChildren () const noexcept
{
    return Iterator {nullptr};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
UniqueString AllocationGroup::GetId () const noexcept
{
    return UniqueString {};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
size_t AllocationGroup::GetAcquired () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
size_t AllocationGroup::GetReserved () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
size_t AllocationGroup::GetTotal () const noexcept
{
    return 0u;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It is special empty implementation.
uintptr_t AllocationGroup::Hash () const noexcept
{
    return 0u;
}

AllocationGroup &AllocationGroup::operator= (const AllocationGroup &_other) noexcept = default;

AllocationGroup &AllocationGroup::operator= (AllocationGroup &&_other) noexcept = default;

bool AllocationGroup::operator== (const AllocationGroup & /*unused*/) const noexcept
{
    return true;
}
bool AllocationGroup::operator!= (const AllocationGroup & /*unused*/) const noexcept
{
    return false;
}

AllocationGroup::AllocationGroup (void *_handle) noexcept : handle (_handle)
{
}
} // namespace Emergence::Memory::Profiler
