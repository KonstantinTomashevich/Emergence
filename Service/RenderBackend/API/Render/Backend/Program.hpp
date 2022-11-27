#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/Vector.hpp>

namespace Emergence::Render::Backend
{
class Program final
{
public:
    static const char *GetShaderSuffix () noexcept;

    Program (const Container::Vector<uint8_t> &_vertexShaderData,
             const Container::Vector<uint8_t> &_fragmentShaderData) noexcept;

    Program (const Program &_other) = delete;

    Program (Program &&_other) noexcept;

    ~Program () noexcept;

    bool IsValid () const noexcept;

    Program &operator= (const Program &_other) = delete;

    Program &operator= (Program &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Render::Backend
