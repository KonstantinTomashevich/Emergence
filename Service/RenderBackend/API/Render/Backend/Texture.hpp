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
    /// \brief Constructs default invalid instance.
    Texture () noexcept;

    /// \brief Constructs texture from given data. Data format is implementation-dependant.
    Texture (const uint8_t *_data, std::uint64_t _size, const TextureSettings &_settings) noexcept;

    /// \brief Constructs texture from given RGBA32 data.
    /// \details Data ownership is not transferred to texture. Instead, data is copied.
    Texture (const uint8_t *_data,
             std::uint64_t _width,
             std::uint64_t _height,
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
    friend class Uniform;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t));
};
} // namespace Emergence::Render::Backend
