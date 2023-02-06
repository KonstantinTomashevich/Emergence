#pragma once

#include <StandardLayout/Mapping.hpp>

struct LevelsConfigurationSingleton final
{
    Emergence::Memory::UniqueString tutorialLevelName;
    Emergence::Memory::UniqueString campaignLevelPrefix;
    uint32_t campaignLevelCount = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId tutorialLevelName;
        Emergence::StandardLayout::FieldId campaignLevelPrefix;
        Emergence::StandardLayout::FieldId campaignLevelCount;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};