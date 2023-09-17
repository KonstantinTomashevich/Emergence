#include <Celerity/Render/2d/DebugShape2dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
DebugShape2d::DebugShape2d (float _boxHalfWidth, float _boxHalfHeight) noexcept
    : type (DebugShape2dType::BOX),
      boxHalfExtents (_boxHalfWidth, _boxHalfHeight)
{
}

DebugShape2d::DebugShape2d (float _circleRadius) noexcept
    : type (DebugShape2dType::CIRCLE),
      circleRadius (_circleRadius)
{
}

DebugShape2d::DebugShape2d (const Math::Vector2f &_lineEnd) noexcept
    : type (DebugShape2dType::LINE),
      lineEnd (_lineEnd)
{
}

const DebugShape2d::Reflection &DebugShape2d::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DebugShape2d);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (boxHalfExtents);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (circleRadius);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lineEnd);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const DebugShape2dComponent::Reflection &DebugShape2dComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DebugShape2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (debugShapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialInstanceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (translation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rotation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shape);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
