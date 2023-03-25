#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Math/Matrix3x3f.hpp>
#include <Math/Matrix4x4f.hpp>
#include <Math/Vector4f.hpp>

#include <Render/Backend/Program.hpp>
#include <Render/Backend/TransientIndexBuffer.hpp>
#include <Render/Backend/TransientVertexBuffer.hpp>
#include <Render/Backend/Uniform.hpp>
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

/// \brief Tells renderer that current draw primitive is triangle.
constexpr std::uint64_t STATE_PRIMITIVE_TRIANGLES = 0x0000000000000000u;

/// \brief Tells renderer that current draw primitive is line.
constexpr std::uint64_t STATE_PRIMITIVE_LINES = 0x0002000000000000u;

/// \brief Provides API for thread safe submission of render commands.
/// \details Usually we want to prepare render commands for independent viewports in parallel.
///          For example, one thread will do the world rendering while another one will do UI
///          rendering. Submission agents provide means to achieve this with minimal locking.
/// \invariant Submission agents should never be cached and should be properly destructed after use.
class SubmissionAgent final
{
public:
    SubmissionAgent (const SubmissionAgent &_other) = delete;

    SubmissionAgent (SubmissionAgent &&_other) noexcept;

    ~SubmissionAgent () noexcept;

    /// \brief Sets given value to given uniform.
    /// \invariant Uniform was constructed with UniformType::VECTOR_4F.
    void SetVector4f (UniformId _uniform, const Math::Vector4f &_value) noexcept;

    /// \brief Sets given value to given uniform.
    /// \invariant Uniform was constructed with UniformType::MATRIX_3X3F.
    void SetMatrix3x3f (UniformId _uniform, const Math::Matrix3x3f &_value) noexcept;

    /// \brief Sets given value to given uniform.
    /// \invariant Uniform was constructed with UniformType::MATRIX_4X4F.
    void SetMatrix4x4f (UniformId _uniform, const Math::Matrix4x4f &_value) noexcept;

    /// \brief Submits given texture to given stage using given uniform.
    /// \invariant Uniform was constructed with UniformType::SAMPLER.
    void SetSampler (UniformId _uniform, uint8_t _stage, TextureId _texture) noexcept;

    /// \brief Sets screen rect scissor for the next submit.
    void SetScissor (uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height) noexcept;

    /// \brief Sets current state that controls what is written and how. See STATE_* constants.
    void SetState (uint64_t _state) noexcept;

    /// \brief Submits given geometry to given viewport that will be rendered using given program.
    void SubmitGeometry (ViewportId _viewport,
                         ProgramId _program,
                         const TransientVertexBuffer &_vertices,
                         const TransientIndexBuffer &_indices) noexcept;

    /// \brief Custom version of SubmitGeometry that allows user to submit
    ///        parts of the buffers by specifying custom offset and count.
    void SubmitGeometry (ViewportId _viewport,
                         ProgramId _program,
                         const TransientVertexBuffer &_vertices,
                         uint32_t _verticesOffset,
                         uint32_t _verticesCount,
                         const TransientIndexBuffer &_indices,
                         uint32_t _indicesOffset,
                         uint32_t _indicesCount) noexcept;

    /// \brief Informs backend that given viewport is still in use, even if no geometries were submitted to it.
    /// \details Needed to trigger internal procedures like color and depth clearing.
    void Touch (ViewportId _viewport) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (SubmissionAgent);

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    explicit SubmissionAgent (void *_pointer) noexcept;
};

/// \brief Stores render state and provides API for submitting geometries to rendering.
class Renderer final
{
public:
    Renderer () noexcept;

    Renderer (const Renderer &_other) = delete;

    Renderer (Renderer &&_other) = delete;

    ~Renderer () noexcept;

    /// \brief Creates new submission agent for thread that needs it.
    /// \details Thread safe.
    SubmissionAgent BeginSubmission () noexcept;

    /// \brief Submits order in which viewports should be renderer in case of overlap.
    /// \warning Not thread safe.
    void SubmitViewportOrder (const Container::Vector<ViewportId> &_viewports) noexcept;

    /// \brief Informs backend that all render commands for given frame were executed and result can be presented.
    /// \warning Not thread safe.
    void SubmitFrame () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Renderer);

private:
    friend class Viewport;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 5u);
};
} // namespace Emergence::Render::Backend
