#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Render::Backend
{
/// \brief Describes type of sampling along texture coordinate.
enum class TextureSampling
{
    NONE = 0,
    MIRROR,
    CLAMP,
    BORDER,
    SHIFT
};

/// \brief Describes which data is stored inside texture and how it is stored.
enum class TextureFormat
{
    // No alpha.
    RGB8 = 0,

    // Full color.
    RGBA8,

    // Depth,
    D16,
    D24,
    D32,
};

/// \brief Contains various supported settings that dictate how texture should be used.
struct TextureSettings final
{
    /// \brief Sampling along U coordinate line.
    TextureSampling uSampling = TextureSampling::NONE;

    /// \brief Sampling along V coordinate line.
    TextureSampling vSampling = TextureSampling::NONE;

    /// \brief Sampling along W coordinate line.
    TextureSampling wSampling = TextureSampling::NONE;

    struct Reflection final
    {
        StandardLayout::FieldId uSampling;
        StandardLayout::FieldId vSampling;
        StandardLayout::FieldId wSampling;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Unique identifier used to reference existing Texture.
using TextureId = std::uint64_t;

/// \brief Represents loaded texture that can be submitted to sampler.
class Texture final
{
public:
    static Texture CreateInvalid () noexcept;

    /// \brief Creates texture from file data.
    /// \details PNG must be supported by any implementation, other formats are optional as of now.
    static Texture CreateFromFile (const std::uint8_t *_data,
                                   std::uint64_t _size,
                                   const TextureSettings &_settings) noexcept;

    /// \brief Creates texture from raw data: array of pixels in the same format as texture should be.
    static Texture CreateFromRaw (std::uint64_t _width,
                                  std::uint64_t _height,
                                  TextureFormat _format,
                                  const std::uint8_t *_data,
                                  const TextureSettings &_settings) noexcept;

    /// \brief Creates texture as frame buffer render target.
    static Texture CreateRenderTarget (std::uint64_t _width,
                                       std::uint64_t _height,
                                       TextureFormat _format,
                                       const TextureSettings &_settings) noexcept;

    Texture (const Texture &_other) = delete;

    Texture (Texture &&_other) noexcept;

    ~Texture () noexcept;

    /// \return Whether texture is successfully loaded and ready to be used.
    [[nodiscard]] bool IsValid () const noexcept;

    /// \brief Unique identifier of this texture.
    [[nodiscard]] TextureId GetId () const noexcept;

    Texture &operator= (const Texture &_other) = delete;

    Texture &operator= (Texture &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));

    Texture (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::Render::Backend
