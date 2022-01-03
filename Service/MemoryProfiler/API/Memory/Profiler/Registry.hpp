#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
class GroupPrefix final
{
public:
    GroupPrefix (UniqueString _id) noexcept;

    ~GroupPrefix () noexcept;
};

template <typename T, typename... Args>
T ConstructWithinGroup (UniqueString _groupId, Args... _args)
{
    GroupPrefix prefix {_groupId};
    return T {std::forward<Args> (_args)...};
}

class Registry final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, Registry);

    private:
        /// Registry constructs iterators.
        friend class Registry;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    static Iterator BeginGlobal () noexcept;

    static Iterator EndGlobal () noexcept;

    /// TODO: If _id is empty, special unprofilable stub registry will be created.
    ///       Required because of event capture, otherwise event creation would fall into infinite loop.
    ///       Is there any better solution to this?
    explicit Registry (UniqueString _id) noexcept;

    Registry (const Registry &_other) noexcept;

    Registry (Registry &&_other) noexcept;

    ~Registry () noexcept;

    void Allocate (size_t _bytesCount) noexcept;

    void Acquire (size_t _bytesCount) noexcept;

    void Release (size_t _bytesCount) noexcept;

    void Free (size_t _bytesCount) noexcept;

    Iterator BeginChildren () noexcept;

    Iterator EndChildren () noexcept;

    UniqueString GetId () const noexcept;

    size_t GetAcquired () const noexcept;

    size_t GetReserved () const noexcept;

    size_t GetTotal () const noexcept;

    Registry &operator= (const Registry &_other) noexcept;

    Registry &operator= (Registry &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

    explicit Registry (void *_handle) noexcept;
};
} // namespace Emergence::Memory::Profiler
