#include <cstdlib>

#include <Assert/Assert.hpp>

#include <Memory/Original/Stack.hpp>

namespace Emergence::Memory::Original
{
Stack::Stack (Profiler::AllocationGroup _group, std::size_t _capacity) noexcept
    : start (malloc (_capacity)),
      group (std::move (_group))
{
    group.Allocate (_capacity);
    end = static_cast<std::uint8_t *> (start) + _capacity;
    head = start;
}

Stack::Stack (Stack &&_other) noexcept
    : start (_other.start),
      end (_other.end),
      head (_other.head),
      group (std::move (_other.group))
{
    _other.start = nullptr;
    _other.end = nullptr;
    _other.head = nullptr;
}

Stack::~Stack () noexcept
{
    group.Release (
        static_cast<std::size_t> (reinterpret_cast<std::uintptr_t> (head) - reinterpret_cast<std::uintptr_t> (start)));
    group.Free (
        static_cast<std::size_t> (reinterpret_cast<std::uintptr_t> (end) - reinterpret_cast<std::uintptr_t> (start)));
    free (start);
}

void *Stack::Acquire (std::size_t _chunkSize, std::uintptr_t _alignAs) noexcept
{
    EMERGENCE_ASSERT (_chunkSize <= GetFreeSize ());
    if (std::uintptr_t alignmentStep = reinterpret_cast<std::uintptr_t> (head) % _alignAs)
    {
        const std::uintptr_t step = _alignAs - alignmentStep;
        head = static_cast<std::uint8_t *> (head) + step;
        group.Acquire (static_cast<std::size_t> (step));
    }

    void *allocated = head;
    head = static_cast<std::uint8_t *> (head) + _chunkSize;
    group.Acquire (_chunkSize);
    return allocated;
}

const void *Stack::Head () const noexcept
{
    return head;
}

void Stack::Release (const void *_newHead) noexcept
{
    EMERGENCE_ASSERT (_newHead >= start);
    EMERGENCE_ASSERT (_newHead <= head);

    group.Release (static_cast<std::size_t> (reinterpret_cast<std::uintptr_t> (head) -
                                             reinterpret_cast<std::uintptr_t> (_newHead)));
    head = const_cast<void *> (_newHead);
}

void Stack::Clear () noexcept
{
    group.Release (
        static_cast<std::size_t> (reinterpret_cast<std::uintptr_t> (head) - reinterpret_cast<std::uintptr_t> (start)));
    head = start;
}

std::size_t Stack::GetFreeSize () const noexcept
{
    return static_cast<std::uint8_t *> (end) - static_cast<std::uint8_t *> (head);
}

const Profiler::AllocationGroup &Stack::GetAllocationGroup () const noexcept
{
    return group;
}
} // namespace Emergence::Memory::Original
