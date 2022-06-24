#pragma once

#include <Math/Constants.hpp>
#include <Math/Transform3d.hpp>

#include <StandardLayout/Mapping.hpp>

struct FollowCameraSettingsSingleton final
{
    Emergence::Math::Transform3d cameraLocalTransform {
        {0.0f, 5.0f, -2.0f}, {{Emergence::Math::PI / 3.0f, 0.0f, 0.0f}}, Emergence::Math::Vector3f::ONE};

    struct Reflection
    {
        Emergence::StandardLayout::FieldId cameraLocalTransform;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
