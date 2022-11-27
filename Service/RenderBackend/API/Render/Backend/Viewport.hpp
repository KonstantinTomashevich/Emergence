#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Math/Transform2d.hpp>

namespace Emergence::Render::Backend
{
enum class ViewportSortMode
{
    SEQUENTIAL = 0u,
    DEPTH_ASCENDING,
    DEPTH_DESCENDING,
};

class Viewport final
{
public:
    Viewport (class Renderer *_context) noexcept;

    Viewport (const Viewport &_other) = delete;

    Viewport (Viewport &&_other) noexcept;

    ~Viewport () noexcept;

    void SubmitConfiguration (std::uint32_t _x,
                              std::uint32_t _y,
                              std::uint32_t _width,
                              std::uint32_t _height,
                              ViewportSortMode _sortMode,
                              std::uint32_t _clearColor) noexcept;

    void SubmitOrthographicView (const Math::Transform2d &_view, const Math::Vector2f &_halfOrthographicSize);

    Viewport &operator= (const Viewport &_other) = delete;

    Viewport &operator= (Viewport &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Render::Backend
