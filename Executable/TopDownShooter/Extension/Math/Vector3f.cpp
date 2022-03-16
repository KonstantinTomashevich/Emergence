#include <Math/Scalar.hpp>
#include <Math/Vector3f.hpp>

namespace Emergence::Math
{
const Vector3f Vector3f::ZERO {0.0f, 0.0f, 0.0f};
const Vector3f Vector3f::ONE {1.0f, 1.0f, 1.0f};

const Vector3f Vector3f::UP {0.0f, 1.0f, 0.0f};
const Vector3f Vector3f::DOWN {0.0f, -1.0f, 0.0f};

const Vector3f Vector3f::RIGHT {1.0f, 0.0f, 0.0f};
const Vector3f Vector3f::LEFT {-1.0f, 0.0f, 0.0f};

const Vector3f Vector3f::FORWARD {0.0f, 0.0f, 1.0f};
const Vector3f Vector3f::BACKWARD {0.0f, 0.0f, -1.0f};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Vector3f::Vector3f () noexcept
{
}

Vector3f::Vector3f (float _x, float _y, float _z) noexcept : x (_x), y (_y), z (_z)
{
}

float Vector3f::Length () const noexcept
{
    return SquareRoot (LengthSquared ());
}

float Vector3f::LengthSquared () const noexcept
{
    return x * x + y * y + z * z;
}

Vector3f &Vector3f::Normalize () noexcept
{
    return *this /= Length ();
}

Vector3f &Vector3f::NormalizeSafe (float _epsilon) noexcept
{
    const float length = Length ();
    if (length > _epsilon)
    {
        *this /= length;
    }
    else
    {
        *this = ZERO;
    }

    return *this;
}

#define IMPLEMENT_OPERATOR(Operator)                                                                                   \
    Vector3f Vector3f::operator Operator (const Vector3f &_other) const noexcept                                       \
    {                                                                                                                  \
        return {x Operator _other.x, y Operator _other.y, z Operator _other.z};                                        \
    }

#define IMPLEMENT_ASSIGNMENT_OPERATOR(Operator)                                                                        \
    Vector3f &Vector3f::operator Operator##= (const Vector3f &_other) noexcept                                         \
    {                                                                                                                  \
        x Operator## = _other.x;                                                                                       \
        y Operator## = _other.y;                                                                                       \
        z Operator## = _other.z;                                                                                       \
        return *this;                                                                                                  \
    }

IMPLEMENT_OPERATOR (+)
IMPLEMENT_ASSIGNMENT_OPERATOR (+)
IMPLEMENT_OPERATOR (-)
IMPLEMENT_ASSIGNMENT_OPERATOR (-)
IMPLEMENT_OPERATOR (*)
IMPLEMENT_ASSIGNMENT_OPERATOR (*)
IMPLEMENT_OPERATOR (/)
IMPLEMENT_ASSIGNMENT_OPERATOR (/)

#define IMPLEMENT_SCALAR_OPERATOR(Operator)                                                                            \
    Vector3f Vector3f::operator Operator (float _scalar) const noexcept                                                \
    {                                                                                                                  \
        return {x Operator _scalar, y Operator _scalar, z Operator _scalar};                                           \
    }

#define IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR(Operator)                                                                 \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): It can not be done here. */                                         \
    Vector3f &Vector3f::operator Operator##= (float _scalar) noexcept                                                  \
    {                                                                                                                  \
        x Operator## = _scalar;                                                                                        \
        y Operator## = _scalar;                                                                                        \
        z Operator## = _scalar;                                                                                        \
        return *this;                                                                                                  \
    }

IMPLEMENT_SCALAR_OPERATOR (+)
IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR (+)
IMPLEMENT_SCALAR_OPERATOR (-)
IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR (-)
IMPLEMENT_SCALAR_OPERATOR (*)
IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR (*)
IMPLEMENT_SCALAR_OPERATOR (/)
IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR (/)

float Dot (const Vector3f &_first, const Vector3f &_second) noexcept
{
    return _first.x * _second.x + _first.y * _second.y + _first.z * _second.z;
}

Vector3f Cross (const Vector3f &_first, const Vector3f &_second) noexcept
{
    return {
        _first.y * _second.z - _first.z * _second.y,
        _first.z * _second.x - _first.x * _second.z,
        _first.x * _second.y - _first.y * _second.x,
    };
}

float UnsignedAngle (const Vector3f &_from, const Vector3f &_to) noexcept
{
    const float cos = Dot (_from, _to) / (_from.Length () * _to.Length ());
    return ACos (Clamp (cos, 0.0f, PI));
}

float SignedAngle (const Vector3f &_from, const Vector3f &_to, const Vector3f &_axis) noexcept
{
    const float angle = UnsignedAngle (_from, _to);
    const Vector3f cross = Cross (_from, _to);

    if (cross.x * _axis.x + cross.y * _axis.y + cross.z * _axis.z < 0.0f)
    {
        return -angle;
    }

    return angle;
}

Vector3f Lerp (const Vector3f &_begin, const Vector3f &_end, float _t) noexcept
{
    return _begin * (1.0f - _t) + _end * _t;
}
} // namespace Emergence::Math
