#pragma once

#include <cstdint>

namespace Emergence::Celerity
{
/// \brief Code of the key, localized to user keyboard layout.
using KeyCode = std::int32_t;

/// \brief Physical layout-independent code of the key.
using ScanCode = std::int32_t;

/// \brief Represents which qualifier keys are active.
using QualifiersMask = std::uint32_t;

/// \brief Represents state of the key: whether it is up or down.
enum class KeyState : uint8_t
{
    DOWN = 0u,
    UP
};
} // namespace Emergence::Celerity
