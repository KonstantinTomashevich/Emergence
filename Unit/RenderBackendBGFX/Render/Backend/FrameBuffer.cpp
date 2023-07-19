#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <Container/Vector.hpp>

#include <Render/Backend/FrameBuffer.hpp>

namespace Emergence::Render::Backend
{
FrameBuffer FrameBuffer::CreateInvalid () noexcept
{
    return {array_cast<std::uint16_t, sizeof (data)> (bgfx::kInvalidHandle)};
}

FrameBuffer::FrameBuffer (FrameBuffer &&_other) noexcept
    : data (_other.data)
{
    block_cast<std::uint16_t> (_other.data) = bgfx::kInvalidHandle;
}

FrameBuffer::~FrameBuffer () noexcept
{
    if (auto nativeId = block_cast<std::uint16_t> (data); nativeId != bgfx::kInvalidHandle)
    {
        bgfx::destroy (bgfx::FrameBufferHandle {nativeId});
    }
}

bool FrameBuffer::IsValid () const noexcept
{
    return block_cast<std::uint16_t> (data) != bgfx::kInvalidHandle;
}

FrameBufferId FrameBuffer::GetId () const noexcept
{
    return block_cast<std::uint16_t> (data);
}

FrameBuffer &FrameBuffer::operator= (FrameBuffer &&_other) noexcept
{
    if (this != &_other)
    {
        this->~FrameBuffer ();
        new (this) FrameBuffer (std::move (_other));
    }

    return *this;
}

FrameBuffer::FrameBuffer (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
    : data (_data)
{
}

struct FrameBufferBuilderInternal final
{
    Container::Vector<bgfx::TextureHandle> renderTargets {Memory::Profiler::AllocationGroup {
        Memory::Profiler::AllocationGroup::Top (), Memory::UniqueString {"FrameBufferBuilder"}}};
};

FrameBufferBuilder::FrameBufferBuilder () noexcept
{
    new (&data) FrameBufferBuilderInternal;
}

FrameBufferBuilder::FrameBufferBuilder (FrameBufferBuilder &&_other) noexcept
{
    new (&data) FrameBufferBuilderInternal {std::move (block_cast<FrameBufferBuilderInternal> (_other.data))};
}

FrameBufferBuilder::~FrameBufferBuilder () noexcept
{
    block_cast<FrameBufferBuilderInternal> (data).~FrameBufferBuilderInternal ();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Not for all implementations, also there is assertion.
void FrameBufferBuilder::Begin () noexcept
{
    EMERGENCE_ASSERT (block_cast<FrameBufferBuilderInternal> (data).renderTargets.empty ());
}

void FrameBufferBuilder::AddRenderTarget (const Texture &_texture) noexcept
{
    block_cast<FrameBufferBuilderInternal> (data).renderTargets.push_back (
        {static_cast<std::uint16_t> (_texture.GetId ())});
}

FrameBuffer FrameBufferBuilder::End () noexcept
{
    auto &internal = block_cast<FrameBufferBuilderInternal> (data);
    const bgfx::FrameBufferHandle handle = bgfx::createFrameBuffer (
        static_cast<std::uint8_t> (internal.renderTargets.size ()), internal.renderTargets.data (), false);

    internal.renderTargets.clear ();
    return {array_cast<std::uint16_t, sizeof (FrameBuffer::data)> (handle.idx)};
}
} // namespace Emergence::Render::Backend
