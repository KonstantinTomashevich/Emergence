#include <bgfx/bgfx.h>

#include <bx/math.h>

#include <Render/Backend/RendererData.hpp>
#include <Render/Backend/Viewport.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
Viewport::Viewport () noexcept
{
    block_cast<uint64_t> (data) = std::numeric_limits<std::uint64_t>::max ();
}

Viewport::Viewport (class Renderer &_context) noexcept
{
    block_cast<uint64_t> (data) = reinterpret_cast<RendererData *> (&_context)->viewportIndexCounter++;
}

Viewport::Viewport (Viewport &&_other) noexcept
    : data (_other.data)
{
}

Viewport::~Viewport () noexcept = default;

void Viewport::SubmitConfiguration (std::uint32_t _x,
                                    std::uint32_t _y,
                                    std::uint32_t _width,
                                    std::uint32_t _height,
                                    ViewportSortMode _sortMode,
                                    std::uint32_t _clearColor) noexcept
{
    auto nativeId = static_cast<uint16_t> (block_cast<uint64_t> (data));
    bgfx::resetView (nativeId);
    bgfx::setViewRect (nativeId, _x, _y, _width, _height);

    uint32_t clearFlags = 0u;
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
}

void Viewport::SubmitOrthographicView (const Math::Transform2d &_view,
                                       const Math::Vector2f &_halfOrthographicSize) noexcept
{
    auto nativeId = static_cast<uint16_t> (block_cast<uint64_t> (data));
    float view[16u];
    bx::mtxSRT (view, _view.scale.x, _view.scale.y, 1.0f, 0.0f, 0.0f, _view.rotation, -_view.translation.x,
                -_view.translation.y, 1.0f);

    float projection[16];
    bx::mtxOrtho (projection, -_halfOrthographicSize.x, _halfOrthographicSize.x, -_halfOrthographicSize.y,
                  _halfOrthographicSize.y, 0.5f, 2.0f, 0.0f, bgfx::getCaps ()->homogeneousDepth);
    bgfx::setViewTransform (nativeId, view, projection);
}

Viewport &Viewport::operator= (Viewport &&_other) noexcept
{
    data = _other.data;
    return *this;
}
} // namespace Emergence::Render::Backend