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
        /// AllocationGroup constructs iterators.
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

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    [[nodiscard]] Iterator EndChildren () const noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    [[nodiscard]] uintptr_t Hash () const noexcept;

    AllocationGroup &operator= (const AllocationGroup &_other) noexcept;

    AllocationGroup &operator= (AllocationGroup &&_other) noexcept;

    bool operator== (const AllocationGroup &_other) const noexcept;

    bool operator!= (const AllocationGroup &_other) const noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

    explicit AllocationGroup (void *_handle) noexcept;
};
} // namespace Emergence::Memory::Profiler

namespace std
{
template <>
struct hash<Emergence::Memory::Profiler::AllocationGroup>
{
    std::size_t operator() (const Emergence::Memory::Profiler::AllocationGroup &_group) const noexcept
    {
        return static_cast<std::size_t> (_group.Hash ());
    }
};
} // namespace std
