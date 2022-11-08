#include <Math/AxisAlignedBox2d.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Math
{
AxisAlignedBox2d::AxisAlignedBox2d (const NoInitializationFlag &_noInitializationFlag) noexcept
    : min (_noInitializationFlag),
      max (_noInitializationFlag)
{
}

AxisAlignedBox2d::AxisAlignedBox2d (const Vector2f &_min, const Vector2f &_max) noexcept
    : min (_min),
      max (_max)
{
}

const AxisAlignedBox2d::Reflection &AxisAlignedBox2d::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AxisAlignedBox2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (min);
        EMERGENCE_MAPPING_REGISTER_REGULAR (max);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

AxisAlignedBox2d Combine (const AxisAlignedBox2d &_first, const AxisAlignedBox2d &_second) noexcept
{
    return {{std::min (_first.min.x, _second.min.x), std::min (_first.min.y, _second.min.y)},
            {std::max (_first.max.x, _second.max.x), std::max (_first.max.y, _second.max.y)}};
}

AxisAlignedBox2d operator* (const Matrix3x3f &_transformMatrix, const AxisAlignedBox2d &_box) noexcept
{
    std::array<Vector2f, 4u> probes {Vector2f {_box.min.x, _box.min.y},
                                     {_box.max.x, _box.min.y},
                                     {_box.max.x, _box.max.y},
                                     {_box.min.x, _box.max.y}};

    AxisAlignedBox2d result {{std::numeric_limits<float>::max (), std::numeric_limits<float>::max ()},
                             {std::numeric_limits<float>::lowest (), std::numeric_limits<float>::lowest ()}};

    for (const Vector2f &probe : probes)
    {
        const Vector3f transformed3f = _transformMatrix * Vector3f {probe.x, probe.y, 1.0f};
        const Vector2f transformed {transformed3f.x, transformed3f.y};

        result.min.x = std::min (result.min.x, transformed.x);
        result.min.y = std::min (result.min.y, transformed.y);

        result.max.x = std::max (result.max.x, transformed.x);
        result.max.y = std::max (result.max.y, transformed.y);
    }

    return result;
}
} // namespace Emergence::Math
