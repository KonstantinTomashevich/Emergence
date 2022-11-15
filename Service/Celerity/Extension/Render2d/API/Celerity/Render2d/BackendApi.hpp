#pragma once

#include <cstdint>

namespace Emergence::Celerity
{
enum class MultiSampleAntiAliasingType
{
    NONE,
    X2,
    X4,
    X8,
    X16
};

struct Render2dBackendConfig final
{
    uint32_t width = 0u;
    uint32_t height = 0u;
    MultiSampleAntiAliasingType msaa = MultiSampleAntiAliasingType::NONE;
    bool vsync = false;
    bool hdr10 = false;
    bool hiDpi = false;
};

struct Render2dBackend final
{
    Render2dBackend () = delete;

    static bool Init (const Render2dBackendConfig &_config,
                      void *_nativeWindowHandle,
                      void *_nativeDisplayType,
                      bool _profileMemory) noexcept;

    static const Render2dBackendConfig &GetCurrentConfig () noexcept;

    static bool Update (const Render2dBackendConfig &_config) noexcept;

    static void TakePngScreenshot (const char *_outputFilePath) noexcept;

    static void Shutdown () noexcept;
};
} // namespace Emergence::Celerity
