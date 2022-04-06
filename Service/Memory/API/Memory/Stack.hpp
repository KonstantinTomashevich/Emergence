#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory
{
/// \brief Allocator, that manages preallocated memory block through stack-like interface.
/// \details Memory stacks are great for allocation of trivially destructible objects with shared lifetime,
///          for example events fired during current frame or global unique strings, due of ability to free
///          all chunks, allocated after some checkpoint, by simply moving stack head to previous position.
class Stack final
{
public:
    /// \param _capacity Stack capacity in bytes.
    explicit Stack (Profiler::AllocationGroup _group, size_t _capacity) noexcept;

    Stack (const Stack &_other) = delete;

    /// \brief Captures memory from given stack, leaving it unusable.
    Stack (Stack &&_other) noexcept;

    ~Stack () noexcept;

    /// \brief Acquires chunk of memory with given size from the stack head.
    /// \param _alignAs Required alignment for requested chunk.
    [[nodiscard]] void *Acquire (size_t _chunkSize, uintptr_t _alignAs) noexcept;

    /// \return Current stack head, that points to first free byte.
    [[nodiscard]] const void *Head () const noexcept;

    /// \brief Moves stack head to given position, marking all bytes after it as free.
    /// \invariant Given head must be previously obtained from ::Head and must be less than current ::Head.
    void Release (const void *_newHead) noexcept;

    /// \brief Resets stack head to initial position, marking all bytes as free.
    void Clear () noexcept;

    /// \return How many free bytes left?
    /// \details Not based on memory profiler implementation, therefore guaranteed to be always correct.
    [[nodiscard]] size_t GetFreeSize () const noexcept;

    /// \return Allocation group to which this allocator belongs.
    /// \warning Group will report zero memory usage if it is a placeholder or
    ///          if executable is linked to no-profile implementation.
    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept;

    /// \brief Copy assigning stack contradicts with its usage practices.
    Stack &operator= (const Stack &_other) = delete;

    /// \brief Captures memory from given stack, leaving it unusable.
    Stack &operator= (Stack &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 4u);
};
} // namespace Emergence::Memory
