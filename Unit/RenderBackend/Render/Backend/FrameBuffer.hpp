#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Render::Backend
{
/// \brief Unique identifier used to reference existing FrameBuffer.
using FrameBufferId = std::uint64_t;

/// \brief Represents frame buffer which primary function is to connect Viewport to its render targets.
class FrameBuffer final
{
public:
    /// \brief Creates invalid instance of the frame buffer.
    /// \details Invalid instance is usually processed as pointer to target window back buffer.
    static FrameBuffer CreateInvalid () noexcept;

    FrameBuffer (const FrameBuffer &_other) = delete;

    FrameBuffer (FrameBuffer &&_other) noexcept;

    ~FrameBuffer () noexcept;

    /// \return Whether frame buffer is successfully created and ready to be used.
    [[nodiscard]] bool IsValid () const noexcept;

    /// \brief Unique identifier of this frame buffer.
    [[nodiscard]] FrameBufferId GetId () const noexcept;

    FrameBuffer &operator= (const FrameBuffer &_other) = delete;

    FrameBuffer &operator= (FrameBuffer &&_other) noexcept;

private:
    friend class FrameBufferBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));

    FrameBuffer (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};

/// \brief Helper class for building multiple render target frame buffers.
class FrameBufferBuilder final
{
public:
    /// \brief Creates new frame buffer builder instance.
    FrameBufferBuilder () noexcept;

    FrameBufferBuilder (const FrameBufferBuilder &_other) = delete;

    FrameBufferBuilder (FrameBufferBuilder &&_other) noexcept;

    ~FrameBufferBuilder () noexcept;

    /// \brief Begins frame buffer building pipeline.
    /// \invariant Frame buffer building pipeline is not active yet.
    void Begin () noexcept;

    /// \brief Adds given texture as frame buffer render target.
    /// \invariant Frame buffer building pipeline is active.
    void AddRenderTarget (const Texture &_texture) noexcept;

    /// \brief Finishes frame buffer building pipeline and creates result frame buffer.
    /// \invariant Frame buffer building pipeline is active.
    FrameBuffer End () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (FrameBufferBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 4u);
};
} // namespace Emergence::Render::Backend
