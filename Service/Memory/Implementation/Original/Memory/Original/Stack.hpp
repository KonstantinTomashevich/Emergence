#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

namespace Emergence::Memory::Original
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

    EMERGENCE_DELETE_ASSIGNMENT (Stack);

private:
    void *start = nullptr;
    void *end = nullptr;
    void *top = nullptr;
};
} // namespace Emergence::Memory::Original
