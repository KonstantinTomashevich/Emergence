#include <cmath>

#include <Math/Constants.hpp>
#include <Math/Scalar.hpp>

namespace Emergence::Math
{
float Sin (float _rad) noexcept
{
    return sinf (_rad);
}

float Cos (float _rad) noexcept
{
    return cosf (_rad);
}

float ASin (float _value) noexcept
{
    return asinf (_value);
}

float ACos (float _value) noexcept
{
    return acosf (_value);
}

float ATan2 (float _y, float _x) noexcept
{
    return atan2 (_y, _x);
}

float Abs (float _value) noexcept
{
    return _value < 0.0f ? -_value : _value;
}

float SquareRoot (float _value) noexcept
{
    return sqrtf (_value);
}

float Clamp (float _value, float _min, float _max) noexcept
{
    return _value < _min ? _min : (_value > _max ? _max : _value);
}

float Lerp (float _begin, float _end, float _t) noexcept
{
    return _begin * (1.0f - _t) + _end * _t;
}

float Truncate (float _value) noexcept
{
    return truncf (_value);
}

float ToDegrees (float _radians) noexcept
{
    float result = _radians * 180.0f / PI;
    result = result - Truncate (result / 360.0f) * 360.0f;
    return result;
}

float NormalizeAngle (float _radians) noexcept
{
    if (_radians > PI || _radians < -PI)
    {
        _radians -= PI * Truncate (_radians / PI);
    }

    return _radians;
}

bool NearlyEqual (float _first, float _second) noexcept
{
    const float difference = _first - _second;
    return difference > -EPSILON && difference < EPSILON;
}
} // namespace Emergence::Math
