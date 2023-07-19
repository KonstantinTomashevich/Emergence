#pragma once

#include <MathApi.hpp>

#include <Math/Transform2d.hpp>
#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Math
{
struct MathApi AxisAlignedBox2d final
{
    AxisAlignedBox2d (const NoInitializationFlag &_noInitializationFlag) noexcept;

    AxisAlignedBox2d (const Vector2f &_min, const Vector2f &_max) noexcept;

    Vector2f min;
    Vector2f max;

    struct MathApi Reflection final
    {
        StandardLayout::FieldId min;
        StandardLayout::FieldId max;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

MathApi AxisAlignedBox2d Combine (const AxisAlignedBox2d &_first, const AxisAlignedBox2d &_second) noexcept;

MathApi AxisAlignedBox2d operator* (const Matrix3x3f &_transformMatrix, const AxisAlignedBox2d &_box) noexcept;
} // namespace Emergence::Math
