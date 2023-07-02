#include <limits>

#include <bgfx/bgfx.h>

#include <bx/math.h>

#include <Render/Backend/RendererData.hpp>
#include <Render/Backend/Viewport.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
Viewport::Viewport () noexcept
{
    block_cast<std::uint64_t> (data) = std::numeric_limits<std::uint64_t>::max ();
}

Viewport::Viewport (class Renderer &_context) noexcept
{
    block_cast<std::uint64_t> (data) = reinterpret_cast<RendererData *> (&_context)->viewportIndexCounter++;
}

Viewport::Viewport (Viewport &&_other) noexcept
    : data (_other.data)
{
    block_cast<std::uint64_t> (_other.data) = std::numeric_limits<std::uint64_t>::max ();
}

Viewport::~Viewport () noexcept = default;

void Viewport::SubmitConfiguration (const FrameBuffer &_frameBuffer,
                                    std::uint32_t _x,
                                    std::uint32_t _y,
                                    std::uint32_t _width,
                                    std::uint32_t _height,
                                    ViewportSortMode _sortMode,
                                    std::uint32_t _clearColor) noexcept
{
    auto nativeId = static_cast<std::uint16_t> (block_cast<std::uint64_t> (data));
    bgfx::resetView (nativeId);
    bgfx::setViewRect (nativeId, static_cast<std::uint16_t> (_x), static_cast<std::uint16_t> (_y),
                       static_cast<std::uint16_t> (_width), static_cast<std::uint16_t> (_height));

    std::uint16_t clearFlags = 0u;
    if (_clearColor)
    {
        clearFlags |= BGFX_CLEAR_COLOR;
    }

    switch (_sortMode)
    {
    case ViewportSortMode::SEQUENTIAL:
        bgfx::setViewMode (nativeId, bgfx::ViewMode::Sequential);
        break;

    case ViewportSortMode::DEPTH_ASCENDING:
        bgfx::setViewMode (nativeId, bgfx::ViewMode::DepthAscending);
        clearFlags |= BGFX_CLEAR_DEPTH;
        break;

    case ViewportSortMode::DEPTH_DESCENDING:
        bgfx::setViewMode (nativeId, bgfx::ViewMode::DepthDescending);
        clearFlags |= BGFX_CLEAR_DEPTH;
        break;
    }

    bgfx::setViewClear (nativeId, clearFlags, _clearColor, 0.0f, 0u);
    bgfx::setViewFrameBuffer (nativeId, {static_cast<std::uint16_t> (_frameBuffer.GetId ())});
}

void Viewport::SubmitOrthographicView (const Math::Transform2d &_view,
                                       const Math::Vector2f &_halfOrthographicSize) noexcept
{
    auto nativeId = static_cast<std::uint16_t> (block_cast<std::uint64_t> (data));
    float view[16u];
    bx::mtxSRT (view, _view.scale.x, _view.scale.y, 1.0f, 0.0f, 0.0f, _view.rotation, -_view.translation.x,
                -_view.translation.y, 1.0f);

    float projection[16];
    bx::mtxOrtho (projection, -_halfOrthographicSize.x, _halfOrthographicSize.x, -_halfOrthographicSize.y,
                  _halfOrthographicSize.y, 0.5f, 2.0f, 0.0f, bgfx::getCaps ()->homogeneousDepth);
    bgfx::setViewTransform (nativeId, view, projection);
}

ViewportId Viewport::GetId () const noexcept
{
    return block_cast<std::uint64_t> (data);
}

Viewport &Viewport::operator= (Viewport &&_other) noexcept
{
    data = _other.data;
    return *this;
}
} // namespace Emergence::Render::Backend
