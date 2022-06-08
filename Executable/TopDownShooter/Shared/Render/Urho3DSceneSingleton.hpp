#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <API/Common/Shortcuts.hpp>

#include <StandardLayout/Mapping.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Scene/Scene.h>
END_MUTING_WARNINGS

struct Urho3DSceneSingleton final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Urho3DSceneSingleton);

    Urho3D::Scene *scene = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId scene;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
