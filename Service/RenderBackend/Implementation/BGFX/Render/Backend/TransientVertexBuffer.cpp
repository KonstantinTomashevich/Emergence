#include <bgfx/bgfx.h>

#include <Render/Backend/TransientVertexBuffer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
uint32_t TransientVertexBuffer::TruncateSizeToAvailability (std::uint32_t _vertexCount, const VertexLayout &_layout)
{
    return bgfx::getAvailTransientVertexBuffer (_vertexCount, block_cast<bgfx::VertexLayout> (_layout.data));
}

TransientVertexBuffer::TransientVertexBuffer (std::uint32_t _vertexCount, const VertexLayout &_layout) noexcept
{
    bgfx::allocTransientVertexBuffer (&block_cast<bgfx::TransientVertexBuffer> (data), _vertexCount,
                                      block_cast<bgfx::VertexLayout> (_layout.data));
}

TransientVertexBuffer::~TransientVertexBuffer () noexcept = default;

void *TransientVertexBuffer::GetData () noexcept
{
    return block_cast<bgfx::TransientVertexBuffer> (data).data;
}

const void *TransientVertexBuffer::GetData () const noexcept
{
    return block_cast<bgfx::TransientVertexBuffer> (data).data;
}
} // namespace Emergence::Render::Backend
