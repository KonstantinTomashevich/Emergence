#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Scene/Node.h>
END_MUTING_WARNINGS

struct Urho3DNodeComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Urho3DNodeComponent);

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Urho3D::Node *node = nullptr;
    std::size_t usages = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId node;
        Emergence::StandardLayout::FieldId usages;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
