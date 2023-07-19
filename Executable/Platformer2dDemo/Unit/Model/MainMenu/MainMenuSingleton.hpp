#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi MainMenuSingleton final
{
    Emergence::Celerity::UniqueId uiListenerObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId uiListenerObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
