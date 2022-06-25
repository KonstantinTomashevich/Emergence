#include <Math/Easing.hpp>

namespace Emergence::Math
{
float EaseInQuint (float _value) noexcept
{
    return _value * _value * _value * _value * _value;
}

float EaseOutQuint (float _value) noexcept
{
    return 1.0f - EaseInQuint (1.0f - _value);
}
} // namespace Emergence::Math
