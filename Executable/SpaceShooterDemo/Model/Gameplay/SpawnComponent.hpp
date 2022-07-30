#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/InplaceVector.hpp>

#include <StandardLayout/Mapping.hpp>

struct SpawnComponent final
{
    static constexpr std::size_t MAX_OBJECTS_PER_SPAWN = 4u;

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString objectToSpawnId;

    uint64_t spawnCoolDownNs = 5000000000u; // 5 seconds.

    uint64_t spawnCoolingDownUntilNs = 0u;

    uint8_t maxSpawnedObjects = 1u;

    Emergence::Container::InplaceVector<Emergence::Celerity::UniqueId, MAX_OBJECTS_PER_SPAWN> spawnedObjects;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId objectToSpawnId;
        Emergence::StandardLayout::FieldId spawnCoolDownNs;
        Emergence::StandardLayout::FieldId spawnCoolingDownUntilNs;
        Emergence::StandardLayout::FieldId maxSpawnedObjects;
        Emergence::StandardLayout::FieldId spawnedObjects;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
