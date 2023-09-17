#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi LevelsConfigurationSingleton final
{
    Emergence::Memory::UniqueString tutorialLevelName;
    Emergence::Memory::UniqueString campaignLevelPrefix;
    std::uint32_t campaignLevelCount = 0u;
    bool loaded = false;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId tutorialLevelName;
        Emergence::StandardLayout::FieldId campaignLevelPrefix;
        Emergence::StandardLayout::FieldId campaignLevelCount;
        Emergence::StandardLayout::FieldId loaded;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
