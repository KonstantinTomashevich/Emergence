#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi MovementConfigurationSingleton final
{
    float jumpActuationThreshold = 0.5f;

    float crouchOrSlideActuationThreshold = 0.5f;

    float runActuationThreshold = 0.1f;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId jumpActuationThreshold;
        Emergence::StandardLayout::FieldId crouchOrSlideActuationThreshold;
        Emergence::StandardLayout::FieldId runActuationThreshold;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
