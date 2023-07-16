#include <Math/Constants.hpp>
#include <Math/Scalar.hpp>
#include <Math/Vector2f.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
const Vector2f Vector2f::ZERO {0.0f, 0.0f};
const Vector2f Vector2f::ONE {1.0f, 1.0f};

const Vector2f Vector2f::RIGHT {1.0f, 0.0f};
const Vector2f Vector2f::LEFT {-1.0f, 0.0f};

const Vector2f Vector2f::UP {0.0f, 1.0f};
const Vector2f Vector2f::DOWN {0.0f, -1.0f};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Vector2f::Vector2f (const NoInitializationFlag & /*unused*/) noexcept
{
}

Vector2f::Vector2f (float _x, float _y) noexcept
    : x (_x),
      y (_y)
{
}

float Vector2f::Length () const noexcept
{
    return SquareRoot (LengthSquared ());
}

float Vector2f::LengthSquared () const noexcept
{
    return x * x + y * y;
}

Vector2f &Vector2f::Normalize () noexcept
{
    return *this /= Length ();
}

Vector2f &Vector2f::NormalizeSafe () noexcept
{
    const float length = Length ();
    if (length > EPSILON)
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
    Vector2f Vector2f::operator Operator (const Vector2f &_other) const noexcept                                       \
    {                                                                                                                  \
        return {x Operator _other.x, y Operator _other.y};                                                             \
    }

#define IMPLEMENT_ASSIGNMENT_OPERATOR(Operator)                                                                        \
    Vector2f &Vector2f::operator Operator##= (const Vector2f &_other) noexcept                                         \
    {                                                                                                                  \
        x Operator## = _other.x;                                                                                       \
        y Operator## = _other.y;                                                                                       \
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
    Vector2f Vector2f::operator Operator (float _scalar) const noexcept                                                \
    {                                                                                                                  \
        return {x Operator _scalar, y Operator _scalar};                                                               \
    }

#define IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR(Operator)                                                                 \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): It can not be done here. */                                         \
    Vector2f &Vector2f::operator Operator##= (float _scalar) noexcept                                                  \
    {                                                                                                                  \
        x Operator## = _scalar;                                                                                        \
        y Operator## = _scalar;                                                                                        \
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

Vector2f Vector2f::operator- () const noexcept
{
    return {-x, -y};
}

const Vector2f::Reflection &Vector2f::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Vector2f);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

float Dot (const Vector2f &_first, const Vector2f &_second) noexcept
{
    return _first.x * _second.x + _first.y * _second.y;
}

float SignedAngle (const Vector2f &_from, const Vector2f &_to) noexcept
{
    return NormalizeAngle (ATan2 (_to.y, _to.x) - ATan2 (_from.y, _from.x));
}

Vector2f Lerp (const Vector2f &_begin, const Vector2f &_end, float _t) noexcept
{
    return _begin * (1.0f - _t) + _end * _t;
}

bool NearlyEqual (const Vector2f &_first, const Vector2f &_second) noexcept
{
    return NearlyEqual (_first.x, _second.x) && NearlyEqual (_first.y, _second.y);
}

Vector2f Rotate (const Vector2f &_vector, float _angleRad) noexcept
{
    const float sin = Sin (_angleRad);
    const float cos = Cos (_angleRad);
    return {_vector.x * cos - _vector.y * sin, _vector.x * sin + _vector.y * cos};
}
} // namespace Emergence::Math
