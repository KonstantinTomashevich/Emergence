#pragma once

#include <cstdint>

namespace Emergence::Render::Backend
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
struct Config final
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

/// \return Allocation group, used for all allocations made by render backend.
Memory::Profiler::AllocationGroup GetSharedAllocationGroup () noexcept;

/// \brief Initializes rendering backend.
/// \invariant Must be called only once per application execution.
bool Init (const Config &_config,
                  void *_nativeWindowHandle,
                  void *_nativeDisplayType,
                  bool _profileMemory) noexcept;

/// \return Last config passed to ::Init or ::Update.
const Config &GetCurrentConfig () noexcept;

/// \brief Updates backend configuration and applies new parameters.
/// \invariant ::Init returned `true`.
bool Update (const Config &_config) noexcept;

/// \brief Takes screenshot of the whole window and saves it to the given path in PNG format.
/// \invariant ::Init returned `true`.
void TakePngScreenshot (const char *_outputFilePath) noexcept;

/// \brief Destructs rendering backend and deallocates its data.
/// \brief Must be called only once per application execution.
/// \invariant ::Init returned `true`.
void Shutdown () noexcept;
} // namespace Emergence::Render::Backend
