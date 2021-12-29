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

    ~Stack () noexcept;

    [[nodiscard]] void *Acquire (size_t _chunkSize, uintptr_t _alignAs = sizeof (uintptr_t)) noexcept;

    [[nodiscard]] const void *Head () const noexcept;

    void Release (const void *_newHead) noexcept;

    void Clear () noexcept;

    [[nodiscard]] size_t GetFreeSize () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Stack);

private:
    void *start = nullptr;
    void *end = nullptr;
    void *head = nullptr;
};
} // namespace Emergence::Memory::Original
