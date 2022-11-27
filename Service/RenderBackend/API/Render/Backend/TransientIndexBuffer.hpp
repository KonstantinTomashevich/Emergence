#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Render::Backend
{
class TransientIndexBuffer
{
public:
    static uint32_t TruncateSizeToAvailability (uint32_t _size, bool _use32BitIndices);

    TransientIndexBuffer (uint32_t _size, bool _use32BitIndices) noexcept;

    TransientIndexBuffer (const TransientIndexBuffer &_other) = delete;

    TransientIndexBuffer (TransientIndexBuffer &&_other) = delete;

    ~TransientIndexBuffer () noexcept;

    void *GetData () noexcept;

    [[nodiscard]] const void *GetData () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TransientIndexBuffer);

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);
};
} // namespace Emergence::Render::Backend
