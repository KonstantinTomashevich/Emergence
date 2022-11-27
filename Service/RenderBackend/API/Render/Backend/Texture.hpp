#pragma once

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Render::Backend
{
/// \brief Represents loaded texture that can be submitted to sampler.
class Texture final
{
public:
    /// \brief Constructs default invalid instance.
    Texture () noexcept;

    /// \brief Constructs texture from given data. Data format is implementation-dependant.
    Texture (const uint8_t *_data, std::uint64_t _size) noexcept;

    Texture (const Texture &_other) = delete;

    Texture (Texture &&_other) noexcept;

    ~Texture () noexcept;

    /// \return Whether texture is successfully loaded and ready to be used.
    [[nodiscard]] bool IsValid () const noexcept;

    Texture &operator= (const Texture &_other) = delete;

    Texture &operator= (Texture &&_other) noexcept;

private:
    friend class Uniform;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t));
};
} // namespace Emergence::Render::Backend
