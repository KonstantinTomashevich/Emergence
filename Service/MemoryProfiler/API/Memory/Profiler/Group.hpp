#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

namespace Emergence::Memory::Profiler
{
class GroupRegistry final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, GroupRegistry);

    private:
        /// GroupRegistry constructs iterators.
        friend class OrderedPool;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    explicit GroupRegistry (const char *_group) noexcept;

    GroupRegistry (const GroupRegistry &_other) noexcept;

    GroupRegistry (GroupRegistry &&_other) noexcept;

    ~GroupRegistry () noexcept;

    void Allocate (size_t _bytesCount) noexcept;

    void Acquire (size_t _bytesCount) noexcept;

    void Release (size_t _bytesCount) noexcept;

    void Free (size_t _bytesCount) noexcept;

    Iterator BeginChildren () noexcept;

    Iterator EndChildren () noexcept;

    const char *GetId () const noexcept;

    size_t GetAcquired () const noexcept;

    size_t GetReserved () const noexcept;

    size_t GetTotal () const noexcept;

    GroupRegistry &operator= (const GroupRegistry &_other) noexcept;

    GroupRegistry &operator= (GroupRegistry &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

    explicit GroupRegistry (void *_handle) noexcept;
};
} // namespace Emergence::Memory::Profiler
