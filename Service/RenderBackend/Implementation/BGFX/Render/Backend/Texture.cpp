#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <bimg/decode.h>

#include <Log/Log.hpp>

#include <Render/Backend/Allocator.hpp>
#include <Render/Backend/Texture.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
Texture::Texture () noexcept
{
    block_cast<uint16_t> (data) = bgfx::kInvalidHandle;
}

static void ImageReleaseCallback (void * /*unused*/, void *_userData)
{
    bimg::imageFree (static_cast<bimg::ImageContainer *> (_userData));
}

Texture::Texture (const uint8_t *_data, const std::uint64_t _size) noexcept
{
    auto &resultHandle = block_cast<uint16_t> (data);
    resultHandle = bgfx::kInvalidHandle;

    bimg::ImageContainer *imageContainer =
        bimg::imageParse (GetCurrentAllocator (), _data, static_cast<uint32_t> (_size));

    if (!imageContainer)
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to parse texture data!");
        return;
    }

    // TODO: Support cube maps and mips in future.
    EMERGENCE_ASSERT (!imageContainer->m_cubeMap);
    EMERGENCE_ASSERT (imageContainer->m_depth == 1u);

    if (!bgfx::isTextureValid (0, false, imageContainer->m_numLayers,
                               static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format),
                               BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to parse texture, because it's data is invalid.");
        return;
    }

    const bgfx::Memory *memory =
        bgfx::makeRef (imageContainer->m_data, imageContainer->m_size, ImageReleaseCallback, imageContainer);

    bgfx::TextureHandle handle = bgfx::createTexture2D (
        static_cast<uint16_t> (imageContainer->m_width), static_cast<uint16_t> (imageContainer->m_height),
        1u < imageContainer->m_numMips, imageContainer->m_numLayers,
        static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
        memory);

    if (!bgfx::isValid (handle))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Failed to create texture!");
        return;
    }

    resultHandle = handle.idx;
}

Texture::Texture (Texture &&_other) noexcept
{
    data = _other.data;
    block_cast<uint16_t> (_other.data) = bgfx::kInvalidHandle;
}

Texture::~Texture () noexcept
{
    if (std::uint16_t handle = block_cast<uint16_t> (data); handle != bgfx::kInvalidHandle)
    {
        bgfx::destroy (bgfx::TextureHandle {handle});
    }
}

bool Texture::IsValid () const noexcept
{
    return block_cast<uint16_t> (data) != bgfx::kInvalidHandle;
}

TextureId Texture::GetId () const noexcept
{
    return static_cast<uint64_t> (block_cast<uint16_t> (data));
}

Texture &Texture::operator= (Texture &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Texture ();
        new (this) Texture (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Render::Backend
