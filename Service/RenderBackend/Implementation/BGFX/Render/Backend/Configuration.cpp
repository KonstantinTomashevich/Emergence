#include <SyntaxSugar/MuteWarnings.hpp>

#include <Assert/Assert.hpp>

#include <bgfx/bgfx.h>

#include <bimg/bimg.h>

BEGIN_MUTING_WARNINGS
#include <bx/file.h>
#include <bx/string.h>
END_MUTING_WARNINGS

#include <Container/StringBuilder.hpp>

#include <Log/Log.hpp>

#include <Memory/Heap.hpp>

#include <Render/Backend/Allocator.hpp>
#include <Render/Backend/Configuration.hpp>

namespace Emergence::Render::Backend
{
static uint32_t ExtractBgfxFlags (const Config &_config)
{
    uint32_t flags = BGFX_RESET_NONE;
    switch (_config.msaa)
    {
    case MultiSampleAntiAliasingType::NONE:
        break;

    case MultiSampleAntiAliasingType::X2:
        flags |= BGFX_RESET_MSAA_X2;
        break;

    case MultiSampleAntiAliasingType::X4:
        flags |= BGFX_RESET_MSAA_X4;
        break;

    case MultiSampleAntiAliasingType::X8:
        flags |= BGFX_RESET_MSAA_X8;
        break;

    case MultiSampleAntiAliasingType::X16:
        flags |= BGFX_RESET_MSAA_X16;
        break;
    }

    if (_config.vsync)
    {
        flags |= BGFX_RESET_VSYNC;
    }

    if (_config.hdr10)
    {
        flags |= BGFX_RESET_HDR10;
    }

    if (_config.hiDpi)
    {
        flags |= BGFX_RESET_HIDPI;
    }

    return flags;
}

struct Callback final : bgfx::CallbackI
{
    Callback () = default;

    Callback (const Callback &_callback) = default;

    Callback (Callback &&_callback) = default;

    ~Callback () override = default;

    void fatal (const char *_filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char *_str) override;

    void traceVargs (const char *_filePath, uint16_t _line, const char *_format, va_list _argList) override;

    void profilerBegin (const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override;

    void profilerBeginLiteral (const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override;

    void profilerEnd () override;

    uint32_t cacheReadSize (uint64_t _id) override;

    bool cacheRead (uint64_t _id, void *_data, uint32_t _size) override;

    void cacheWrite (uint64_t _id, const void *_data, uint32_t _size) override;

    void screenShot (const char *_filePath,
                     uint32_t _width,
                     uint32_t _height,
                     uint32_t _pitch,
                     const void *_data,
                     uint32_t _size,
                     bool _yFlip) override;

    void captureBegin (
        uint32_t _width, uint32_t _height, uint32_t _pitch, bgfx::TextureFormat::Enum _format, bool _yFlip) override;

    void captureEnd () override;

    void captureFrame (const void *_data, uint32_t _size) override;

    Callback &operator= (const Callback &_callback) = default;

    Callback &operator= (Callback &&_callback) = default;
};

void Callback::fatal (const char *_filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char *_str)
{
    if (_code == bgfx::Fatal::DebugCheck)
    {
        // Do not report debug checks as fatal. Instead, just log them as debug.
        EMERGENCE_LOG (DEBUG, "Render::Backend: BGFX debug check failed: ", _str, " (File: \"", _filePath,
                       "\", Line: \"", _line, "\")");
        return;
    }

    ReportCriticalError (EMERGENCE_BUILD_STRING ("Render::Backend: BGFX Error with code ",
                                                 static_cast<std::underlying_type_t<bgfx::Fatal::Enum>> (_code),
                                                 " and message \"", _str, "\"."),
                         _filePath, _line);
}

void Callback::traceVargs (const char *_filePath, uint16_t _line, const char *_format, va_list _argList)
{
    char buffer[2048u];
    char *lastCharacter = buffer;
    va_list argListCopy;
    va_copy (argListCopy, _argList);
    std::size_t length = bx::snprintf (lastCharacter, sizeof (buffer), "%s (%d): ", _filePath, _line);
    std::size_t total = length + bx::vsnprintf (lastCharacter + length, static_cast<int32_t> (sizeof (buffer) - length),
                                                _format, argListCopy);
    va_end (argListCopy);

    EMERGENCE_ASSERT (sizeof (buffer) >= total);

    // Remove unnecessary line end.
    if (total > 0u && lastCharacter[total - 1u] == '\n')
    {
        --total;
    }

    lastCharacter[total] = '\0';
    EMERGENCE_LOG (DEBUG, "Render::Backend: BGFX trace: ", std::string_view {&buffer[0u]});
}

void Callback::profilerBegin (const char * /*unused*/,
                              uint32_t /*unused*/,
                              const char * /*unused*/,
                              uint16_t /*unused*/)
{
    // We do not support BGFX profiling yet.
}

void Callback::profilerBeginLiteral (const char * /*unused*/,
                                     uint32_t /*unused*/,
                                     const char * /*unused*/,
                                     uint16_t /*unused*/)
{
    // We do not support BGFX profiling yet.
}

void Callback::profilerEnd ()
{
    // We do not support BGFX profiling yet.
}

uint32_t Callback::cacheReadSize (uint64_t /*unused*/)
{
    // We do not support BGFX caching yet.
    return 0;
}

bool Callback::cacheRead (uint64_t /*unused*/, void * /*unused*/, uint32_t /*unused*/)
{
    // We do not support BGFX caching yet.
    return false;
}

void Callback::cacheWrite (uint64_t /*unused*/, const void * /*unused*/, uint32_t /*unused*/)
{
    // We do not support BGFX caching yet.
}

void Callback::screenShot (const char *_filePath,
                           uint32_t _width,
                           uint32_t _height,
                           uint32_t _pitch,
                           const void *_data,
                           uint32_t /*unused*/,
                           bool _yFlip)
{
    EMERGENCE_ASSERT (Container::String {_filePath}.ends_with (".png"));
    bx::FileWriter writer;
    bx::Error err;

    if (bx::open (&writer, _filePath, false, &err))
    {
        bimg::imageWritePng (&writer, _width, _height, _pitch, _data, bimg::TextureFormat::BGRA8, _yFlip, &err);
        bx::close (&writer);
    }
}

void Callback::captureBegin (uint32_t /*unused*/,
                             uint32_t /*unused*/,
                             uint32_t /*unused*/,
                             bgfx::TextureFormat::Enum /*unused*/,
                             bool /*unused*/)
{
    // We do not support BGFX capture yet.
}

void Callback::captureEnd ()
{
    // We do not support BGFX capture yet.
}

void Callback::captureFrame (const void * /*unused*/, uint32_t /*unused*/)
{
    // We do not support BGFX capture yet.
}

static Config currentConfig;

bool Init (const Config &_config, void *_nativeWindowHandle, void *_nativeDisplayType, bool _profileMemory) noexcept
{
    currentConfig = _config;
    bgfx::Init init;
    init.type = bgfx::RendererType::Count;
    init.resolution.width = _config.width;
    init.resolution.height = _config.height;

    init.resolution.reset = ExtractBgfxFlags (_config);
    init.platformData.nwh = _nativeWindowHandle;
    init.platformData.ndt = _nativeDisplayType;

    if (_profileMemory)
    {
        SetCurrentAllocator (GetProfiledAllocator ());
    }
    else
    {
        SetCurrentAllocator (GetEfficientAllocator ());
    }

    init.allocator = GetCurrentAllocator ();
    static Callback callback;
    init.callback = &callback;
    return bgfx::init (init);
}

const Config &GetCurrentConfig () noexcept
{
    return currentConfig;
}

bool Update (const Config &_config) noexcept
{
    currentConfig = _config;
    bgfx::reset (_config.width, _config.height, ExtractBgfxFlags (_config));
    return true;
}

void TakePngScreenshot (const char *_outputFilePath) noexcept
{
    bgfx::requestScreenShot (BGFX_INVALID_HANDLE, _outputFilePath);
}

void Shutdown () noexcept
{
    bgfx::shutdown ();
}
} // namespace Emergence::Render::Backend
