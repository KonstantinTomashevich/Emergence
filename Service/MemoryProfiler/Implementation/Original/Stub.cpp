#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/Profiler/Capture.hpp>

namespace Emergence::Memory::Profiler
{
// TODO: Empty stubs are temporary inserted for linking.

AllocationGroup::PlacedOnStack::~PlacedOnStack ()
{
}

AllocationGroup::PlacedOnStack::PlacedOnStack (void *_handle) noexcept : handle (_handle)
{
    handle = _handle;
}

bool AllocationGroup::Iterator::operator== ([[maybe_unused]] const AllocationGroup::Iterator &_other) const noexcept
{
    return false;
}

AllocationGroup::Iterator &AllocationGroup::Iterator::operator++ () noexcept
{
    return *this;
}

AllocationGroup::Iterator AllocationGroup::Iterator::operator++ (int) noexcept
{
    return AllocationGroup::Iterator (nullptr);
}

AllocationGroup AllocationGroup::Iterator::operator* () const noexcept
{
    return AllocationGroup (Memory::UniqueString ());
}

AllocationGroup::Iterator::Iterator ([[maybe_unused]] const AllocationGroup::Iterator &_other) noexcept
{
}

AllocationGroup::Iterator::Iterator ([[maybe_unused]] AllocationGroup::Iterator &&_other) noexcept
{
}

AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (
    [[maybe_unused]] const AllocationGroup::Iterator &_other) noexcept
{
    return *this;
}
AllocationGroup::Iterator &AllocationGroup::Iterator::operator= (
    [[maybe_unused]] AllocationGroup::Iterator &&_other) noexcept
{
    return *this;
}

AllocationGroup::Iterator::~Iterator () noexcept
{
}

bool AllocationGroup::Iterator::operator!= ([[maybe_unused]] const AllocationGroup::Iterator &_other) const noexcept
{
    return false;
}

AllocationGroup::Iterator::Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    data = *_data;
}

AllocationGroup::AllocationGroup ([[maybe_unused]] UniqueString _id) noexcept
{
}

AllocationGroup::AllocationGroup ([[maybe_unused]] const AllocationGroup &_parent,
                                  [[maybe_unused]] UniqueString _id) noexcept
{
}

AllocationGroup::AllocationGroup ([[maybe_unused]] const AllocationGroup &_other) noexcept
{
}

AllocationGroup::AllocationGroup ([[maybe_unused]] AllocationGroup &&_other) noexcept
{
}

AllocationGroup::~AllocationGroup () noexcept
{
}

AllocationGroup AllocationGroup::Root () noexcept
{
    return AllocationGroup ();
}

AllocationGroup AllocationGroup::Top () noexcept
{
    return AllocationGroup ();
}

AllocationGroup::AllocationGroup () noexcept
{
}

AllocationGroup::PlacedOnStack AllocationGroup::PlaceOnTop () const noexcept
{
    return AllocationGroup::PlacedOnStack (nullptr);
}

void AllocationGroup::Allocate ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void AllocationGroup::Acquire ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void AllocationGroup::Release ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

void AllocationGroup::Free ([[maybe_unused]] size_t _bytesCount) noexcept
{
}

AllocationGroup::Iterator AllocationGroup::BeginChildren () const noexcept
{
    return AllocationGroup::Iterator (nullptr);
}

AllocationGroup::Iterator AllocationGroup::EndChildren () const noexcept
{
    return AllocationGroup::Iterator (nullptr);
}

UniqueString AllocationGroup::GetId () const noexcept
{
    return UniqueString ();
}

size_t AllocationGroup::GetAcquired () const noexcept
{
    return 0;
}

size_t AllocationGroup::GetReserved () const noexcept
{
    return 0;
}

size_t AllocationGroup::GetTotal () const noexcept
{
    return 0;
}

AllocationGroup &AllocationGroup::operator= ([[maybe_unused]] const AllocationGroup &_other) noexcept
{
    return *this;
}

AllocationGroup &AllocationGroup::operator= ([[maybe_unused]] AllocationGroup &&_other) noexcept
{
    return *this;
}

AllocationGroup::AllocationGroup (void *_handle) noexcept : handle (_handle)
{
    handle = _handle;
}
} // namespace Emergence::Memory::Profiler
