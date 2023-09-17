#include <API/Common/BlockCast.hpp>

#include <bgfx/bgfx.h>

#include <Render/Backend/TransientIndexBuffer.hpp>

namespace Emergence::Render::Backend
{
uint32_t TransientIndexBuffer::TruncateSizeToAvailability (std::uint32_t _indexCount, bool _use32BitIndices)
{
    // NOLINTNEXTLINE(readability-suspicious-call-argument): It doesn't look easily swappable due to types.
    return bgfx::getAvailTransientIndexBuffer (_indexCount, _use32BitIndices);
}

TransientIndexBuffer::TransientIndexBuffer (std::uint32_t _indexCount, bool _use32BitIndices) noexcept
{
    // NOLINTNEXTLINE(readability-suspicious-call-argument): It doesn't look easily swappable due to types.
    bgfx::allocTransientIndexBuffer (&block_cast<bgfx::TransientIndexBuffer> (data), _indexCount, _use32BitIndices);
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
