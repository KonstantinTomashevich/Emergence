#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct RenderSceneSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (RenderSceneSingleton);

    Emergence::Celerity::UniqueId cameraObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId cameraObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
