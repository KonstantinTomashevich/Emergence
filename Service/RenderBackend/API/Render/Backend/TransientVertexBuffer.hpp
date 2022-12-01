#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Render/Backend/VertexLayout.hpp>

namespace Emergence::Render::Backend
{
/// \brief Temporary vertex buffer that will be automatically destructed after frame is rendered.
/// \details Useful for drawing geometry that might change every frame. For example, batched 2d sprites or UI.
///          Must be thread safe.
class TransientVertexBuffer
{
public:
    /// \brief If there is not enough space to allocate buffer for required count, returns maximum possible count.
    static uint32_t TruncateSizeToAvailability (uint32_t _vertexCount, const VertexLayout &_layout);

    /// \brief Constructs temporary buffer for given count of vertices using given vertex layout.
    TransientVertexBuffer (uint32_t _vertexCount, const VertexLayout &_layout) noexcept;

    TransientVertexBuffer (const TransientVertexBuffer &_other) = delete;

    TransientVertexBuffer (TransientVertexBuffer &&_other) = delete;

    ~TransientVertexBuffer () noexcept; // NOLINT(performance-trivially-destructible): Implementation-specific.

    /// \return Pointer to buffer data.
    void *GetData () noexcept;

    /// \return Pointer to buffer data.
    [[nodiscard]] const void *GetData () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TransientVertexBuffer);

private:
    friend class SubmissionAgent;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);
};
} // namespace Emergence::Render::Backend
