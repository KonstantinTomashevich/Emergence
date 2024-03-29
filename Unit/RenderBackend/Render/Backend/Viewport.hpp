#pragma once

#include <RenderBackendApi.hpp>

#include <API/Common/ImplementationBinding.hpp>

#include <Math/Transform2d.hpp>

#include <Render/Backend/FrameBuffer.hpp>

namespace Emergence::Render::Backend
{
/// \brief Unique identifier used to reference existing Viewport.
using ViewportId = std::uint64_t;

/// \brief Enumerates supported sort modes. Sort mode defines how geometries are sorted inside viewport.
enum class ViewportSortMode
{
    /// \brief Geometries are sorted in the same order they were submitted.
    SEQUENTIAL = 0u,

    /// \brief Geometries are sorted by their depth using ascending order.
    DEPTH_ASCENDING,

    /// \brief Geometries are sorted by their depth using descending order.
    DEPTH_DESCENDING,
};

/// \brief Represents a render surface on target framebuffer to which geometries can be submitted.
/// \details Working with different viewports from different threads is thread safe,
///          but working with one viewport from multiple threads is not.
class RenderBackendApi Viewport final
{
public:
    /// \brief Constructs default invalid instance.
    Viewport () noexcept;

    /// \brief Constructs viewport for given renderer.
    Viewport (class Renderer &_context) noexcept;

    Viewport (const Viewport &_other) = delete;

    Viewport (Viewport &&_other) noexcept;

    ~Viewport () noexcept; // NOLINT(performance-trivially-destructible): Implementation-specific.

    /// \brief Submits viewport configuration.
    /// \param _frameBuffer Target frame buffer, if invalid frame buffer is passed, then target is output window.
    /// \param _x Leftmost coordinate in window coordinate system.
    /// \param _y Topmost coordinate in window coordinate system.
    /// \param _width Viewport width in window coordinate system.
    /// \param _height Viewport height in window coordinate system.
    /// \param _sortMode Sorting algorithm for viewport drawables.
    /// \param _clearColor Color that will be used to clear viewport. Passing 0 disables viewport clearing.
    void SubmitConfiguration (const FrameBuffer &_frameBuffer,
                              std::uint32_t _x,
                              std::uint32_t _y,
                              std::uint32_t _width,
                              std::uint32_t _height,
                              ViewportSortMode _sortMode,
                              std::uint32_t _clearColor) noexcept;

    /// \brief Submits view and projection matrices for this viewport using orthographic projection.
    void SubmitOrthographicView (const Math::Transform2d &_view, const Math::Vector2f &_halfOrthographicSize) noexcept;

    /// \brief Unique identifier of this viewport.
    [[nodiscard]] ViewportId GetId () const noexcept;

    Viewport &operator= (const Viewport &_other) = delete;

    Viewport &operator= (Viewport &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));
};
} // namespace Emergence::Render::Backend
