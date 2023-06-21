#pragma once

#include <cstdint>

struct SDL_Window;

namespace Emergence::Testing
{
/// \brief Initializes and holds SDL window, also initializes RenderBackend inside it.
class SDLContextHolder final
{
public:
    /// \brief Width of window, created by SDLContextHolder.
    static constexpr std::uint32_t WIDTH = 400u;

    /// \brief Height of window, created by SDLContextHolder.
    static constexpr std::uint32_t HEIGHT = 300u;

    /// \return Singleton instance of this class.
    static SDLContextHolder &Get () noexcept;

    /// \brief Advances to next frame by pooling all the system events without reacting to them.
    void Frame () noexcept;

    SDLContextHolder (const SDLContextHolder &_other) = delete;

    SDLContextHolder (SDLContextHolder &&_other) = delete;

    SDLContextHolder &operator= (const SDLContextHolder &_other) = delete;

    SDLContextHolder &operator= (SDLContextHolder &&_other) = delete;

private:
    SDLContextHolder () noexcept;

    ~SDLContextHolder () noexcept;

    SDL_Window *window = nullptr;
};
} // namespace Emergence::Testing
