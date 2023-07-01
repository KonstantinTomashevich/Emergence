#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <bimg/decode.h>

#include <Log/Log.hpp>

#include <Render/Backend/Allocator.hpp>
#include <Render/Backend/Texture.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Render::Backend
{
static std::uint64_t SettingsToFlags (const TextureSettings &_settings)
{
    std::uint64_t flags = BGFX_TEXTURE_NONE;
    switch (_settings.uSampling)
    {
    case TextureSampling::NONE:
        break;

    case TextureSampling::MIRROR:
        flags |= BGFX_SAMPLER_U_MIRROR;
        break;

    case TextureSampling::CLAMP:
        flags |= BGFX_SAMPLER_U_CLAMP;
        break;

    case TextureSampling::BORDER:
        flags |= BGFX_SAMPLER_U_BORDER;
        break;

    case TextureSampling::SHIFT:
        flags |= BGFX_SAMPLER_U_SHIFT;
        break;
    }

    switch (_settings.vSampling)
    {
    case TextureSampling::NONE:
        break;

    case TextureSampling::MIRROR:
        flags |= BGFX_SAMPLER_V_MIRROR;
        break;

    case TextureSampling::CLAMP:
        flags |= BGFX_SAMPLER_V_CLAMP;
        break;

    case TextureSampling::BORDER:
        flags |= BGFX_SAMPLER_V_BORDER;
        break;

    case TextureSampling::SHIFT:
        flags |= BGFX_SAMPLER_V_SHIFT;
        break;
    }

    switch (_settings.wSampling)
    {
    case TextureSampling::NONE:
        break;

    case TextureSampling::MIRROR:
        flags |= BGFX_SAMPLER_W_MIRROR;
        break;

    case TextureSampling::CLAMP:
        flags |= BGFX_SAMPLER_W_CLAMP;
        break;

    case TextureSampling::BORDER:
        flags |= BGFX_SAMPLER_W_BORDER;
        break;

    case TextureSampling::SHIFT:
        flags |= BGFX_SAMPLER_W_SHIFT;
        break;
    }

    return flags;
}

static std::size_t GetFormatElementSize (TextureFormat _format) noexcept
{
    switch (_format)
    {
    case TextureFormat::RGB8:
        return 3u;

    case TextureFormat::RGBA8:
        return 4u;

    case TextureFormat::D16:
        return 2u;

    case TextureFormat::D24:
        return 3u;

    case TextureFormat::D32:
        return 4u;
    }

    EMERGENCE_ASSERT (false);
    return 1u;
}

static bgfx::TextureFormat::Enum ToBgfxFormat (TextureFormat _format)
{
    switch (_format)
    {
    case TextureFormat::RGB8:
        return bgfx::TextureFormat::RGB8;

    case TextureFormat::RGBA8:
        return bgfx::TextureFormat::RGBA8;

    case TextureFormat::D16:
        return bgfx::TextureFormat::D16;

    case TextureFormat::D24:
        return bgfx::TextureFormat::D24;

    case TextureFormat::D32:
        return bgfx::TextureFormat::D32;
    }

    EMERGENCE_ASSERT (false);
    return bgfx::TextureFormat::RGBA8;
}

const TextureSettings::Reflection &TextureSettings::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TextureSettings);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uSampling);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vSampling);
        EMERGENCE_MAPPING_REGISTER_REGULAR (wSampling);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

Texture Texture::CreateInvalid () noexcept
{
    return {array_cast<std::uint16_t, sizeof (data)> (bgfx::kInvalidHandle)};
}

static void ImageReleaseCallback (void * /*unused*/, void *_userData)
{
    bimg::imageFree (static_cast<bimg::ImageContainer *> (_userData));
}

Texture Texture::CreateFromFile (const std::uint8_t *_data,
                                 std::uint64_t _size,
                                 const TextureSettings &_settings) noexcept
{
    bimg::ImageContainer *imageContainer =
        bimg::imageParse (GetCurrentAllocator (), _data, static_cast<std::uint32_t> (_size));

    if (!imageContainer)
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to parse texture data!");
        return CreateInvalid ();
    }

    // TODO: Support cube maps and mips in the future.
    EMERGENCE_ASSERT (!imageContainer->m_cubeMap);
    EMERGENCE_ASSERT (imageContainer->m_depth == 1u);

    if (!bgfx::isTextureValid (0, false, imageContainer->m_numLayers,
                               static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format),
                               BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Unable to parse texture, because it's data is invalid.");
        return CreateInvalid ();
    }

    const bgfx::Memory *memory =
        bgfx::makeRef (imageContainer->m_data, imageContainer->m_size, ImageReleaseCallback, imageContainer);

    bgfx::TextureHandle handle = bgfx::createTexture2D (
        static_cast<std::uint16_t> (imageContainer->m_width), static_cast<std::uint16_t> (imageContainer->m_height),
        1u < imageContainer->m_numMips, imageContainer->m_numLayers,
        static_cast<bgfx::TextureFormat::Enum> (imageContainer->m_format), SettingsToFlags (_settings), memory);

    if (!bgfx::isValid (handle))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Failed to create texture!");
        return CreateInvalid ();
    }

    return {array_cast<std::uint16_t, sizeof (data)> (handle.idx)};
}

Texture Texture::CreateFromRaw (std::uint64_t _width,
                                std::uint64_t _height,
                                TextureFormat _format,
                                const std::uint8_t *_data,
                                const TextureSettings &_settings) noexcept
{
    bgfx::TextureHandle handle = bgfx::createTexture2D (
        static_cast<std::uint16_t> (_width), static_cast<std::uint16_t> (_height), false, 1u, ToBgfxFormat (_format),
        SettingsToFlags (_settings),
        bgfx::copy (_data, static_cast<std::uint32_t> (_width * _height * GetFormatElementSize (_format))));

    if (!bgfx::isValid (handle))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Failed to create texture!");
        return CreateInvalid ();
    }

    return {array_cast<std::uint16_t, sizeof (data)> (handle.idx)};
}

Texture Texture::CreateRenderTarget (std::uint64_t _width,
                                     std::uint64_t _height,
                                     TextureFormat _format,
                                     const TextureSettings &_settings) noexcept
{
    bgfx::TextureHandle handle =
        bgfx::createTexture2D (static_cast<std::uint16_t> (_width), static_cast<std::uint16_t> (_height), false, 1u,
                               ToBgfxFormat (_format), SettingsToFlags (_settings));

    if (!bgfx::isValid (handle))
    {
        EMERGENCE_LOG (ERROR, "Render::Backend: Failed to create texture!");
        return CreateInvalid ();
    }

    return {array_cast<std::uint16_t, sizeof (data)> (handle.idx)};
}

Texture::Texture (Texture &&_other) noexcept
{
    data = _other.data;
    block_cast<std::uint16_t> (_other.data) = bgfx::kInvalidHandle;
}

Texture::~Texture () noexcept
{
    if (std::uint16_t handle = block_cast<std::uint16_t> (data); handle != bgfx::kInvalidHandle)
    {
        bgfx::destroy (bgfx::TextureHandle {handle});
    }
}

bool Texture::IsValid () const noexcept
{
    return block_cast<std::uint16_t> (data) != bgfx::kInvalidHandle;
}

TextureId Texture::GetId () const noexcept
{
    return static_cast<std::uint64_t> (block_cast<std::uint16_t> (data));
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

Texture::Texture (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
    : data (_data)
{
}
} // namespace Emergence::Render::Backend
