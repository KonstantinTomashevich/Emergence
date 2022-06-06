#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct StaticModelComponent final
{
    static constexpr std::size_t MAXIMUM_MATERIAL_SLOTS = 8u;

    EMERGENCE_STATIONARY_DATA_TYPE (StaticModelComponent);

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Memory::UniqueString modelId;
    std::array<Emergence::Memory::UniqueString, MAXIMUM_MATERIAL_SLOTS> materialIds;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId modelId;
        std::array<Emergence::StandardLayout::FieldId, MAXIMUM_MATERIAL_SLOTS> materialIds;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
