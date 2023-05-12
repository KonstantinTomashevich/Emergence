#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct LoadingAnimationSingleton final
{
    bool required = false;
    bool instanced = false;

    Emergence::Celerity::UniqueId cameraObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Celerity::UniqueId sceneObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId required;
        Emergence::StandardLayout::FieldId instanced;
        Emergence::StandardLayout::FieldId cameraObjectId;
        Emergence::StandardLayout::FieldId sceneObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
