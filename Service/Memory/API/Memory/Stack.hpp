#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Memory
{
class Stack final
{
public:
    explicit Stack (size_t _capacity) noexcept;

    Stack (const Stack &_other) = delete;

    Stack (Stack &&_other) noexcept;

    ~Stack ();

    [[nodiscard]] void *Acquire (size_t _chunkSize) noexcept;

    [[nodiscard]] const void *Top () const noexcept;

    void Release (const void *_newTop) noexcept;

    [[nodiscard]] size_t GetFreeSize () const noexcept;

    Stack &operator= (const Stack &_other) = delete;

    Stack &operator= (Stack &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);
};
} // namespace Emergence::Memory
