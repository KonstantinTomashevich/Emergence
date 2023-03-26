#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct SpawnComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString prototypeId;

    uint64_t spawnDelayNs = 0u;

    uint64_t nextSpawnTimeNs = 0u;

    uint8_t maxSpawnedCount = 1u;

    uint8_t currentSpawnedCount = 0u;

    bool respawn = true;

    struct Reflection final
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