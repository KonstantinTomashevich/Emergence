#pragma once

#include <MathApi.hpp>

namespace Emergence::Math
{
MathApi float Sin (float _rad) noexcept;

MathApi float Cos (float _rad) noexcept;

/// \return Angle in radians.
MathApi float ASin (float _value) noexcept;

/// \return Angle in radians.
MathApi float ACos (float _value) noexcept;

/// \return Angle in radians.
MathApi float ATan2 (float _y, float _x) noexcept;

MathApi float Abs (float _value) noexcept;

MathApi float SquareRoot (float _value) noexcept;

MathApi float Clamp (float _value, float _min, float _max) noexcept;

MathApi float Lerp (float _begin, float _end, float _t) noexcept;

MathApi float Truncate (float _value) noexcept;

MathApi float ToDegrees (float _radians) noexcept;

/// \brief Moves angle into [-PI, PI] range.
MathApi float NormalizeAngle (float _radians) noexcept;

MathApi bool NearlyEqual (float _first, float _second) noexcept;
} // namespace Emergence::Math
