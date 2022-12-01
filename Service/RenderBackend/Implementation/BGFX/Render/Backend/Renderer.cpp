#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <Render/Backend/Renderer.hpp>
#include <Render/Backend/RendererData.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
SubmissionAgent::SubmissionAgent (SubmissionAgent &&_other) noexcept
{
    data = _other.data;
    block_cast<bgfx::Encoder *> (_other.data) = nullptr;
}

SubmissionAgent::~SubmissionAgent () noexcept
{
    if (auto *encoder = block_cast<bgfx::Encoder *> (data))
    {
        bgfx::end (encoder);
    }
}

void SubmissionAgent::SetState (uint64_t _state) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    encoder->setState (_state);
}

void SubmissionAgent::SubmitGeometry (ViewportId _viewport,
                                      ProgramId _program,
                                      const TransientVertexBuffer &_vertices,
                                      const TransientIndexBuffer &_indices) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    encoder->setVertexBuffer (0, &block_cast<bgfx::TransientVertexBuffer> (_vertices.data));
    encoder->setIndexBuffer (&block_cast<bgfx::TransientIndexBuffer> (_indices.data));
    encoder->submit (static_cast<uint16_t> (_viewport), bgfx::ProgramHandle {static_cast<uint16_t> (_program)});
}

void SubmissionAgent::Touch (ViewportId _viewport) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    encoder->touch (static_cast<uint16_t> (_viewport));
}

void SubmissionAgent::SetVector4f (UniformId _uniform, const Math::Vector4f &_value) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    EMERGENCE_ASSERT (_uniform != bgfx::kInvalidHandle);
    encoder->setUniform ({static_cast<uint16_t> (_uniform)}, &_value);
}

void SubmissionAgent::SetMatrix3x3f (UniformId _uniform, const Math::Matrix3x3f &_value) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    EMERGENCE_ASSERT (_uniform != bgfx::kInvalidHandle);
    encoder->setUniform ({static_cast<uint16_t> (_uniform)}, &_value);
}

void SubmissionAgent::SetMatrix4x4f (UniformId _uniform, const Math::Matrix4x4f &_value) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    EMERGENCE_ASSERT (_uniform != bgfx::kInvalidHandle);
    encoder->setUniform ({static_cast<uint16_t> (_uniform)}, &_value);
}

void SubmissionAgent::SetSampler (UniformId _uniform, uint8_t _stage, TextureId _texture) noexcept
{
    auto *encoder = block_cast<bgfx::Encoder *> (data);
    EMERGENCE_ASSERT (encoder);
    EMERGENCE_ASSERT (_uniform != bgfx::kInvalidHandle);
    EMERGENCE_ASSERT (_texture != bgfx::kInvalidHandle);
    encoder->setTexture (_stage, {static_cast<uint16_t> (_uniform)}, {static_cast<uint16_t> (_texture)});
}

SubmissionAgent::SubmissionAgent (void *_pointer) noexcept
{
    block_cast<bgfx::Encoder *> (data) = static_cast<bgfx::Encoder *> (_pointer);
}

Renderer::Renderer () noexcept
{
    new (data.data ()) RendererData ();
}

Renderer::~Renderer () noexcept
{
    block_cast<RendererData> (data).~RendererData ();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Being non-static is a part of the API.
SubmissionAgent Renderer::BeginSubmission () noexcept
{
    return SubmissionAgent {bgfx::begin ()};
}

void Renderer::SubmitViewportOrder (const Container::Vector<ViewportId> &_viewports) noexcept
{
    auto &rendererData = block_cast<RendererData> (data);
    rendererData.viewNativeIds.reserve (_viewports.size ());
    uint16_t minimum = std::numeric_limits<uint16_t>::max ();

    for (ViewportId viewport : _viewports)
    {
        rendererData.viewNativeIds.emplace_back (static_cast<uint16_t> (viewport));
        minimum = std::min (minimum, rendererData.viewNativeIds.back ());
    }

    bgfx::setViewOrder (minimum, static_cast<uint16_t> (rendererData.viewNativeIds.size ()),
                        rendererData.viewNativeIds.data ());
    rendererData.viewNativeIds.clear ();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Being non-static is a part of the API.
void Renderer::SubmitFrame () noexcept
{
    bgfx::frame ();
}
} // namespace Emergence::Render::Backend
