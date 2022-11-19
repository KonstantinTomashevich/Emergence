#pragma once

#include <cstdint>

namespace Emergence::Celerity
{
/// \brief Supported MSAA powers.
enum class MultiSampleAntiAliasingType
{
    NONE,
    X2,
    X4,
    X8,
    X16
};

/// \brief Contains configurable parameters that must be supported by 2d rendering backend.
struct Render2dBackendConfig final
{
    /// \brief Window width in pixels.
    uint32_t width = 0u;

    /// \brief Window height in pixels.
    uint32_t height = 0u;

    /// \brief Selected MSAA mode.
    MultiSampleAntiAliasingType msaa = MultiSampleAntiAliasingType::NONE;

    /// \brief Whether VSync is enabled.
    bool vsync = false;

    /// \brief Whether HDR10 is enabled.
    bool hdr10 = false;

    /// \brief Whether HiDPI is enabled.
    bool hiDpi = false;
};

/// \brief Holds static API for 2d rendering backend.
struct Render2dBackend final
{
    Render2dBackend () = delete;

    /// \brief Initializes rendering backend.
    /// \invariant Must be called only once per application execution.
    static bool Init (const Render2dBackendConfig &_config,
                      void *_nativeWindowHandle,
                      void *_nativeDisplayType,
                      bool _profileMemory) noexcept;

    /// \return Last config passed to ::Init or ::Update.
    static const Render2dBackendConfig &GetCurrentConfig () noexcept;

    /// \brief Updates backend configuration and applies new parameters.
    /// \invariant ::Init returned `true`.
    static bool Update (const Render2dBackendConfig &_config) noexcept;

    /// \brief Takes screenshot of the whole window and saves it to the given path in PNG format.
    /// \invariant ::Init returned `true`.
    static void TakePngScreenshot (const char *_outputFilePath) noexcept;

    /// \brief Destructs rendering backend and deallocates its data.
    /// \brief Must be called only once per application execution.
    /// \invariant ::Init returned `true`.
    static void Shutdown () noexcept;
};
} // namespace Emergence::Celerity
