#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/Vector.hpp>

namespace Emergence::Render::Backend
{
class Program final
{
public:
    static const char *GetShaderSuffix () noexcept;

    Program () noexcept;

    Program (const uint8_t *_vertexShaderData, std::uint64_t _vertexShaderSize,
             const uint8_t *_fragmentShaderData, std::uint64_t _fragmentShaderSize) noexcept;

    Program (const Program &_other) = delete;

    Program (Program &&_other) noexcept;

    ~Program () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    Program &operator= (const Program &_other) = delete;

    Program &operator= (Program &&_other) noexcept;

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t));
};
} // namespace Emergence::Render::Backend
