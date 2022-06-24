#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct ParticleEffectComponent final
{
    EMERGENCE_STATIONARY_DATA_TYPE (ParticleEffectComponent);

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Celerity::UniqueId effectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString effectName;
    Emergence::Memory::UniqueString effectTag;
    bool playing = false;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId effectId;
        Emergence::StandardLayout::FieldId effectName;
        Emergence::StandardLayout::FieldId effectTag;
        Emergence::StandardLayout::FieldId playing;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
