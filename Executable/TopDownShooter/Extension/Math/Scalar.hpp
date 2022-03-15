#pragma once

namespace Emergence::Math
{
float Sin (float _rad) noexcept;

float Cos (float _rad) noexcept;

/// \return Angle in radians.
float ASin (float _value) noexcept;

/// \return Angle in radians.
float ACos (float _value) noexcept;

float SquareRoot (float _value) noexcept;

float Clamp (float _value, float _min, float _max) noexcept;

float Lerp (float _begin, float _end, float _t) noexcept;
} // namespace Emergence::Math
