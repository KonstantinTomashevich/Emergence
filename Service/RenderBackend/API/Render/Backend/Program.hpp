#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/Vector.hpp>

namespace Emergence::Render::Backend
{
/// \brief Represents linked render program for rendering geometries.
class Program final
{
public:
    static const char *GetShaderSuffix () noexcept;

    /// \brief Constructs default invalid object.
    Program () noexcept;

    /// \brief Constructs program from given implementation-dependant data of vertex and fragment shaders.
    Program (const uint8_t *_vertexShaderData,
             std::uint64_t _vertexShaderSize,
             const uint8_t *_fragmentShaderData,
             std::uint64_t _fragmentShaderSize) noexcept;

    Program (const Program &_other) = delete;

    Program (Program &&_other) noexcept;

    ~Program () noexcept;

    /// \return Whether program is correctly linked and ready to be used.
    [[nodiscard]] bool IsValid () const noexcept;

    Program &operator= (const Program &_other) = delete;

    Program &operator= (Program &&_other) noexcept;

private:
    friend class Renderer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t));
};
} // namespace Emergence::Render::Backend