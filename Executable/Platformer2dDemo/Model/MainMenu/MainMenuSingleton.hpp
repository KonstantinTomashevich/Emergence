#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct MainMenuSingleton final
{
    Emergence::Celerity::UniqueId uiListenerObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId uiListenerObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
