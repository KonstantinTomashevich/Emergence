#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Render/Backend/Program.hpp>
#include <Render/Backend/TransientIndexBuffer.hpp>
#include <Render/Backend/TransientVertexBuffer.hpp>
#include <Render/Backend/Viewport.hpp>

namespace Emergence::Render::Backend
{
class Viewport;

constexpr std::uint64_t STATE_WRITE_R = 0x0000000000000001u;
constexpr std::uint64_t STATE_WRITE_G = 0x0000000000000002u;
constexpr std::uint64_t STATE_WRITE_B = 0x0000000000000004u;
constexpr std::uint64_t STATE_WRITE_A = 0x0000000000000008u;
constexpr std::uint64_t STATE_WRITE_Z = 0x0000004000000000u;
constexpr std::uint64_t STATE_CULL_CW = 0x0000001000000000u;
constexpr std::uint64_t STATE_BLEND_ALPHA = 0x0000000006565000u;
constexpr std::uint64_t STATE_WRITE_MSAA = 0x0100000000000000u;

class Renderer final
{
public:
    Renderer () noexcept;

    Renderer (const Renderer &_other) = delete;

    Renderer (Renderer &&_other) noexcept;

    ~Renderer () noexcept;

    void SetState (uint64_t _state) noexcept;

    void SubmitGeometry (const Viewport &_viewport,
                         const Program &_program,
                         const TransientVertexBuffer &_vertices,
                         const TransientIndexBuffer &_indices) noexcept;

    void Touch (class Viewport &_viewport) noexcept;

    void SubmitViewportOrder (const Container::Vector<Viewport *> &_viewports) noexcept;

    void SubmitFrame () noexcept;

    Renderer &operator= (const Renderer &_other) = delete;

    Renderer &operator= (Renderer &&_other) noexcept;

private:
    friend class Viewport;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 5u);
};
} // namespace Emergence::Render::Backend
