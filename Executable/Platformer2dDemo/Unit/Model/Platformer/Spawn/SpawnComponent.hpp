#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi SpawnComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString prototypeId;

    std::uint64_t spawnDelayNs = 0u;

    std::uint64_t nextSpawnTimeNs = 0u;

    std::uint8_t maxSpawnedCount = 1u;

    std::uint8_t currentSpawnedCount = 0u;

    bool respawn = true;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId prototypeId;
        Emergence::StandardLayout::FieldId spawnDelayNs;
        Emergence::StandardLayout::FieldId nextSpawnTimeNs;
        Emergence::StandardLayout::FieldId maxSpawnedCount;
        Emergence::StandardLayout::FieldId currentSpawnedCount;
        Emergence::StandardLayout::FieldId respawn;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
