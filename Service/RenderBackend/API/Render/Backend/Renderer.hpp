#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Render/Backend/Program.hpp>
#include <Render/Backend/TransientIndexBuffer.hpp>
#include <Render/Backend/TransientVertexBuffer.hpp>
#include <Render/Backend/Viewport.hpp>

namespace Emergence::Render::Backend
{
class Viewport;

/// \brief Whether red part of color should be written.
constexpr std::uint64_t STATE_WRITE_R = 0x0000000000000001u;

/// \brief Whether green part of color should be written.
constexpr std::uint64_t STATE_WRITE_G = 0x0000000000000002u;

/// \brief Whether blue part of color should be written.
constexpr std::uint64_t STATE_WRITE_B = 0x0000000000000004u;

/// \brief Whether alpha should be written.
constexpr std::uint64_t STATE_WRITE_A = 0x0000000000000008u;

/// \brief Whether depth should be written.
constexpr std::uint64_t STATE_WRITE_Z = 0x0000004000000000u;

/// \brief Whether CW culling is enabled.
constexpr std::uint64_t STATE_CULL_CW = 0x0000001000000000u;

/// \brief Whether alpha blending is enabled.
constexpr std::uint64_t STATE_BLEND_ALPHA = 0x0000000006565000u;

/// \brief Whether multisample antialiasing is enabled.
constexpr std::uint64_t STATE_MSAA = 0x0100000000000000u;

/// \brief Stores render state and provides API for submitting geometries to rendering.
class Renderer final
{
public:
    Renderer () noexcept;

    Renderer (const Renderer &_other) = delete;

    Renderer (Renderer &&_other) noexcept;

    ~Renderer () noexcept;

    /// \brief Sets current state that controls what is written and how. See STATE_* constants.
    void SetState (uint64_t _state) noexcept;

    /// \brief Submits given geometry to given viewport that will be rendered using given program.
    void SubmitGeometry (const Viewport &_viewport,
                         const Program &_program,
                         const TransientVertexBuffer &_vertices,
                         const TransientIndexBuffer &_indices) noexcept;

    /// \brief Informs backend that given viewport is still in use, even if no geometries were submitted to it.
    /// \details Needed to trigger internal procedures like color and depth clearing.
    void Touch (const Viewport &_viewport) noexcept;

    /// \brief Submits order in which viewports should be renderer in case of overlap.
    void SubmitViewportOrder (const Container::Vector<const Viewport *> &_viewports) noexcept;

    /// \brief Informs backend that all render commands for given frame were executed and result can be presented.
    void SubmitFrame () noexcept;

    Renderer &operator= (const Renderer &_other) = delete;

    Renderer &operator= (Renderer &&_other) noexcept;

private:
    friend class Viewport;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 5u);
};
} // namespace Emergence::Render::Backend
