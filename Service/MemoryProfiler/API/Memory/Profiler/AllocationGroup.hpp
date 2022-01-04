#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{

class AllocationGroup final
{
public:
    class PlacedOnStack final
    {
    public:
        ~PlacedOnStack ();

    private:
        /// AllocationGroup constructs stack placeholders.
        friend class AllocationGroup;

        EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

        explicit PlacedOnStack (void *_handle) noexcept;
    };

    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, AllocationGroup);

    private:
        /// AllocationGroupconstructs iterators.
        friend class AllocationGroup;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    static AllocationGroup Root () noexcept;

    static AllocationGroup Top () noexcept;

    /// \brief Creates empty placeholder allocation group, that has zero logic.
    AllocationGroup () noexcept;

    /// \brief Creates new group with given id, that is child of ::Top.
    explicit AllocationGroup (UniqueString _id) noexcept;

    /// \brief Constructs allocation group as child of other given allocation group.
    AllocationGroup (const AllocationGroup &_parent, UniqueString _id) noexcept;

    AllocationGroup (const AllocationGroup &_other) noexcept;

    AllocationGroup (AllocationGroup &&_other) noexcept;

    ~AllocationGroup () noexcept;

    [[nodiscard]] PlacedOnStack PlaceOnTop () const noexcept;

    void Allocate (size_t _bytesCount) noexcept;

    void Acquire (size_t _bytesCount) noexcept;

    void Release (size_t _bytesCount) noexcept;

    void Free (size_t _bytesCount) noexcept;

    // TODO: Const iteration?

    [[nodiscard]] Iterator BeginChildren () noexcept;

    [[nodiscard]] Iterator EndChildren () noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    AllocationGroup &operator= (const AllocationGroup &_other) noexcept;

    AllocationGroup &operator= (AllocationGroup &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

    explicit AllocationGroup (void *_handle) noexcept;
};
} // namespace Emergence::Memory::Profiler
