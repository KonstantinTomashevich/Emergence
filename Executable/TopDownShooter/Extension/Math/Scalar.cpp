#include <cmath>

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
} // namespace Emergence::Math
