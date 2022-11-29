#include <bgfx/bgfx.h>

#include <Render/Backend/Renderer.hpp>
#include <Render/Backend/RendererData.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
Renderer::Renderer () noexcept
{
    new (data.data ()) RendererData ();
}

Renderer::Renderer (Renderer &&_other) noexcept
{
    new (data.data ()) RendererData (std::move (block_cast<RendererData> (_other.data)));
}

Renderer::~Renderer () noexcept
{
    block_cast<RendererData> (data).~RendererData ();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Being non-static is a part of the API.
void Renderer::SetState (uint64_t _state) noexcept
{
    bgfx::setState (_state);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Being non-static is a part of the API.
void Renderer::SubmitGeometry (ViewportId _viewport,
                               ProgramId _program,
                               const TransientVertexBuffer &_vertices,
                               const TransientIndexBuffer &_indices) noexcept
{
    bgfx::setVertexBuffer (0, &block_cast<bgfx::TransientVertexBuffer> (_vertices.data));
    bgfx::setIndexBuffer (&block_cast<bgfx::TransientIndexBuffer> (_indices.data));
    bgfx::submit (static_cast<uint16_t> (_viewport), bgfx::ProgramHandle {static_cast<uint16_t> (_program)});
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Being non-static is a part of the API.
void Renderer::Touch (ViewportId _viewport) noexcept
{
    bgfx::touch (static_cast<uint16_t> (_viewport));
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

Renderer &Renderer::operator= (Renderer &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Renderer ();
        new (this) Renderer (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Render::Backend
