#include <bgfx/bgfx.h>

#include <Render/Backend/TransientIndexBuffer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
uint32_t TransientIndexBuffer::TruncateSizeToAvailability (uint32_t _vertexCount, bool _use32BitIndices)
{
    return bgfx::getAvailTransientIndexBuffer (_vertexCount, _use32BitIndices);
}

TransientIndexBuffer::TransientIndexBuffer (uint32_t _vertexCount, bool _use32BitIndices) noexcept
{
    bgfx::allocTransientIndexBuffer (&block_cast<bgfx::TransientIndexBuffer> (data), _vertexCount, _use32BitIndices);
}

TransientIndexBuffer::~TransientIndexBuffer () noexcept = default;

void *TransientIndexBuffer::GetData () noexcept
{
    return block_cast<bgfx::TransientIndexBuffer> (data).data;
}

const void *TransientIndexBuffer::GetData () const noexcept
{
    return block_cast<bgfx::TransientIndexBuffer> (data).data;
}
} // namespace Emergence::Render::Backend
