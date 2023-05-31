#pragma once

#include <cstdint>

struct SDL_Window;

namespace Emergence::Celerity::Test
{
constexpr std::uint32_t WIDTH = 400u;
constexpr std::uint32_t HEIGHT = 300u;

class ContextHolder final
{
public:
    static void Frame () noexcept;

    ContextHolder (const ContextHolder &_other) = delete;

    ContextHolder (ContextHolder &&_other) = delete;

    ContextHolder &operator= (const ContextHolder &_other) = delete;

    ContextHolder &operator= (ContextHolder &&_other) = delete;

private:
    ContextHolder () noexcept;

    ~ContextHolder () noexcept;

    SDL_Window *window = nullptr;
};
} // namespace Emergence::Celerity::Test
