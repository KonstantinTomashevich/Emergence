#pragma once

#include <cstdint>

namespace Emergence::Celerity
{
/// \brief Enumerates supported mouse buttons.
enum class MouseButton : std::uint8_t
{
    LEFT = 0u,
    MIDDLE,
    RIGHT,
    X1,
    X2
};
} // namespace Emergence::Celerity
