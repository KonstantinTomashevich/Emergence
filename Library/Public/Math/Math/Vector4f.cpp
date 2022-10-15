#include <Math/Constants.hpp>
#include <Math/Scalar.hpp>
#include <Math/Vector4f.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
const Vector4f Vector4f::ZERO {0.0f, 0.0f, 0.0f, 0.0f};
const Vector4f Vector4f::ONE {1.0f, 1.0f, 1.0f, 1.0f};

// NOLINTNEXTLINE(modernize-use-equals-default): We need non-default constructor to omit ::components initialization.
Vector4f::Vector4f (const NoInitializationFlag & /*unused*/) noexcept
{
}

Vector4f::Vector4f (float _x, float _y, float _z, float _w) noexcept
    : x (_x),
      y (_y),
      z (_z),
      w (_w)
{
}

float Vector4f::Length () const noexcept
{
    return SquareRoot (LengthSquared ());
}

float Vector4f::LengthSquared () const noexcept
{
    return x * x + y * y + z * z + w * w;
}

Vector4f &Vector4f::Normalize () noexcept
{
    return *this /= Length ();
}

Vector4f &Vector4f::NormalizeSafe () noexcept
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
    Vector4f Vector4f::operator Operator (const Vector4f &_other) const noexcept                                       \
    {                                                                                                                  \
        return {x Operator _other.x, y Operator _other.y, z Operator _other.z, w Operator _other.w};                   \
    }

#define IMPLEMENT_ASSIGNMENT_OPERATOR(Operator)                                                                        \
    Vector4f &Vector4f::operator Operator##= (const Vector4f &_other) noexcept                                         \
    {                                                                                                                  \
        x Operator## = _other.x;                                                                                       \
        y Operator## = _other.y;                                                                                       \
        z Operator## = _other.z;                                                                                       \
        w Operator## = _other.w;                                                                                       \
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
    Vector4f Vector4f::operator Operator (float _scalar) const noexcept                                                \
    {                                                                                                                  \
        return {x Operator _scalar, y Operator _scalar, z Operator _scalar, w Operator _scalar};                       \
    }

#define IMPLEMENT_SCALAR_ASSIGNMENT_OPERATOR(Operator)                                                                 \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses): It can not be done here. */                                         \
    Vector4f &Vector4f::operator Operator##= (float _scalar) noexcept                                                  \
    {                                                                                                                  \
        x Operator## = _scalar;                                                                                        \
        y Operator## = _scalar;                                                                                        \
        z Operator## = _scalar;                                                                                        \
        w Operator## = _scalar;                                                                                        \
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

const Vector4f::Reflection &Vector4f::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Vector4f);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (z);
        EMERGENCE_MAPPING_REGISTER_REGULAR (w);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

float Dot (const Vector4f &_first, const Vector4f &_second) noexcept
{
    return _first.x * _second.x + _first.y * _second.y + _first.z * _second.z + _first.w * _second.w;
}

Vector4f Lerp (const Vector4f &_begin, const Vector4f &_end, float _t) noexcept
{
    return _begin * (1.0f - _t) + _end * _t;
}

bool NearlyEqual (const Vector4f &_first, const Vector4f &_second) noexcept
{
    return NearlyEqual (_first.x, _second.x) && NearlyEqual (_first.y, _second.y) &&
           NearlyEqual (_first.z, _second.z) && NearlyEqual (_first.w, _second.w);
}
} // namespace Emergence::Math
