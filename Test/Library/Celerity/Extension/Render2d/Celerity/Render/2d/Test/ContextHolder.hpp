#pragma once

#include <Celerity/Render/2d/Test/Scenario.hpp>

struct SDL_Window;

namespace Emergence::Celerity::Test
{
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
