#pragma once

#include <API/Common/Iterator.hpp>

#include <Container/InplaceVector.hpp>

#include <Memory/Profiler/Original/ProfilingLock.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler::Original
{
class CapturedAllocationGroup;

class AllocationGroup final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, AllocationGroup *);

    private:
        /// AllocationGroup constructs iterators.
        friend class AllocationGroup;

        explicit Iterator (AllocationGroup *_current) noexcept;

        AllocationGroup *current;
    };

    static AllocationGroup *Root () noexcept;

    static AllocationGroup *Request (UniqueString _id, const ProfilingLock &_lock) noexcept;

    static AllocationGroup *Request (AllocationGroup *_parent,
                                     UniqueString _id,
                                     const ProfilingLock & /*unused*/) noexcept;

    void Allocate (size_t _bytesCount, const ProfilingLock &_lock) noexcept;

    void Acquire (size_t _bytesCount, const ProfilingLock &_lock) noexcept;

    void Release (size_t _bytesCount, const ProfilingLock &_lock) noexcept;

    void Free (size_t _bytesCount, const ProfilingLock &_lock) noexcept;

    [[nodiscard]] AllocationGroup *Parent () const noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    static Iterator EndChildren () noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    [[nodiscard]] uintptr_t Hash () const noexcept;

private:
    friend class AllocationGroupStack;

    friend class CapturedAllocationGroup;

    AllocationGroup (AllocationGroup *_parent, UniqueString _id) noexcept;

    void AllocateInternal (size_t _bytesCount) noexcept;

    void AcquireInternal (size_t _bytesCount) noexcept;

    void ReleaseInternal (size_t _bytesCount) noexcept;

    void FreeInternal (size_t _bytesCount) noexcept;

    Memory::UniqueString id;
    std::size_t reserved = 0u;
    std::size_t acquired = 0u;

    AllocationGroup *parent = nullptr;
    AllocationGroup *firstChild = nullptr;
    AllocationGroup *nextOnLevel = nullptr;
};

class AllocationGroupStack final
{
public:
    static AllocationGroupStack &Get () noexcept;

    void Push (AllocationGroup *_group) noexcept;

    AllocationGroup *Top () noexcept;

    void Pop () noexcept;

private:
    AllocationGroupStack ();

    Container::InplaceVector<AllocationGroup *, 32u> stack;
};
} // namespace Emergence::Memory::Profiler::Original
