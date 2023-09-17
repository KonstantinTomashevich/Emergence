#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <API/Common/BlockCast.hpp>

#include <bgfx/bgfx.h>

#include <Log/Log.hpp>

#include <Render/Backend/Program.hpp>

namespace Emergence::Render::Backend
{
const char *Program::GetShaderSuffix () noexcept
{
    switch (bgfx::getRendererType ())
    {
    case bgfx::RendererType::Noop:
        return ".noop";

    case bgfx::RendererType::Agc:
        return ".agc";

    case bgfx::RendererType::Direct3D9:
        return ".dx9";

    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12:
        return ".dx11";

    case bgfx::RendererType::Gnm:
        return ".pssl";

    case bgfx::RendererType::Metal:
        return ".metal";

    case bgfx::RendererType::Nvn:
        return ".nvn";

    case bgfx::RendererType::OpenGLES:
        return ".essl";

    case bgfx::RendererType::OpenGL:
        return ".glsl";

    case bgfx::RendererType::Vulkan:
        return ".spirv";

    case bgfx::RendererType::WebGPU:
        return ".wssl";

    case bgfx::RendererType::Count:
        return ".unknown";
    }

    return ".unknown";
}

Program::Program () noexcept
{
    block_cast<std::uint16_t> (data) = bgfx::kInvalidHandle;
}

Program::Program (const std::uint8_t *_vertexShaderData,
                  std::uint64_t _vertexShaderSize,
                  const std::uint8_t *_fragmentShaderData,
                  std::uint64_t _fragmentShaderSize) noexcept
{
    auto &resultHandle = block_cast<std::uint16_t> (data);
    resultHandle = bgfx::kInvalidHandle;

    auto loadShader = [] (const std::uint8_t *_data, std::uint64_t _size)
    {
        const bgfx::Memory *shaderMemory = bgfx::alloc (static_cast<std::uint32_t> (_size + 1u));
        memcpy (shaderMemory->data, _data, _size);
        shaderMemory->data[shaderMemory->size - 1] = '\0';
        return bgfx::createShader (shaderMemory);
    };

    bgfx::ShaderHandle vertexShader = loadShader (_vertexShaderData, _vertexShaderSize);
    if (!bgfx::isValid (vertexShader))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to load vertex shader from given data.");
        return;
    }

    bgfx::ShaderHandle fragmentShader = loadShader (_fragmentShaderData, _fragmentShaderSize);
    if (!bgfx::isValid (fragmentShader))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to load fragment shader from given data.");
        return;
    }

    bgfx::ProgramHandle program = bgfx::createProgram (vertexShader, fragmentShader, true);
    if (!bgfx::isValid (program))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to link program from given shaders.");
        return;
    }

    resultHandle = program.idx;
}

Program::Program (Program &&_other) noexcept
{
    data = _other.data;
    block_cast<std::uint16_t> (_other.data) = bgfx::kInvalidHandle;
}

Program::~Program () noexcept
{
    if (auto handle = block_cast<std::uint16_t> (data); handle != bgfx::kInvalidHandle)
    {
        bgfx::destroy (bgfx::ProgramHandle {handle});
    }
}

bool Program::IsValid () const noexcept
{
    return block_cast<std::uint16_t> (data) != bgfx::kInvalidHandle;
}

ProgramId Program::GetId () const noexcept
{
    return static_cast<std::uint64_t> (block_cast<std::uint16_t> (data));
}

Program &Program::operator= (Program &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Program ();
        new (this) Program (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Render::Backend
