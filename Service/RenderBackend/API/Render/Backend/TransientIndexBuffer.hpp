#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::Render::Backend
{
/// \brief Temporary index buffer that will be automatically destructed after frame is rendered.
/// \details Useful for drawing geometry that might change every frame. For example, batched 2d sprite or UI.
class TransientIndexBuffer
{
public:
    /// \brief If there is not enough space to allocate buffer for required count, returns maximum possible count.
    static uint32_t TruncateSizeToAvailability (uint32_t _indexCount, bool _use32BitIndices);

    /// \brief Constructs temporary buffer for given count of indices using 32-bit indices if requested.
    TransientIndexBuffer (uint32_t _indexCount, bool _use32BitIndices) noexcept;

    TransientIndexBuffer (const TransientIndexBuffer &_other) = delete;

    TransientIndexBuffer (TransientIndexBuffer &&_other) = delete;

    ~TransientIndexBuffer () noexcept;

    /// \return Pointer to buffer data.
    void *GetData () noexcept;

    /// \return Pointer to buffer data.
    [[nodiscard]] const void *GetData () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TransientIndexBuffer);

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);
};
} // namespace Emergence::Render::Backend
