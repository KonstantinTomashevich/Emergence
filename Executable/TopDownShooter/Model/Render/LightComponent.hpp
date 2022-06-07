#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Color.hpp>

enum class LightType
{
    DIRECTIONAL = 0u,
    POINT,
    SPOT
};

struct LightComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (LightComponent);

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Celerity::UniqueId lightId = Emergence::Celerity::INVALID_UNIQUE_ID;

    LightType lightType = LightType::DIRECTIONAL;
    bool enabled = true;

    Color color;
    float range = 1.0f;
    float spotFieldOfViewRad = 1.0f;
    float spotAspectRatio = 1.0f;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId lightId;
        Emergence::StandardLayout::FieldId lightType;
        Emergence::StandardLayout::FieldId enabled;
        Emergence::StandardLayout::FieldId color;
        Emergence::StandardLayout::FieldId range;
        Emergence::StandardLayout::FieldId spotFieldOfViewRad;
        Emergence::StandardLayout::FieldId spotAspectRatio;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
