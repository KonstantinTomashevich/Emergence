#pragma once

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Render/Backend/VertexLayout.hpp>

namespace Emergence::Render::Backend
{
class TransientVertexBuffer
{
public:
    static uint32_t TruncateSizeToAvailability (uint32_t _vertexCount, const VertexLayout &_layout);

    TransientVertexBuffer (uint32_t _vertexCount, const VertexLayout &_layout) noexcept;

    TransientVertexBuffer (const TransientVertexBuffer &_other) = delete;

    TransientVertexBuffer (TransientVertexBuffer &&_other) = delete;

    ~TransientVertexBuffer () noexcept;

    void *GetData () noexcept;

    [[nodiscard]] const void *GetData () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (TransientVertexBuffer);

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);
};
} // namespace Emergence::Render::Backend
