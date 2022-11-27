#pragma once

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::Render::Backend
{
class Texture final
{
public:
    Texture () noexcept;

    Texture (const uint8_t *_data, std::uint64_t _size) noexcept;

    Texture (const Texture &_other) = delete;

    Texture (Texture &&_other) noexcept;

    ~Texture () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    Texture &operator= (const Texture &_other) = delete;

    Texture &operator= (Texture &&_other) noexcept;

private:
    friend class Uniform;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t));
};
} // namespace Emergence::Render::Backend
