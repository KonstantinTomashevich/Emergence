#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Container/InplaceVector.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

struct StaticModelComponent final
{
    static constexpr std::size_t MAXIMUM_MATERIAL_SLOTS = 8u;

    EMERGENCE_STATIONARY_DATA_TYPE (StaticModelComponent);

    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;
    Emergence::Celerity::UniqueId modelId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString modelName;
    Emergence::Container::InplaceVector<Emergence::Memory::UniqueString, MAXIMUM_MATERIAL_SLOTS> materialNames;

    void *implementationHandle = nullptr;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId modelId;
        Emergence::StandardLayout::FieldId modelName;
        Emergence::StandardLayout::FieldId materialNamesBlock;
        std::array<Emergence::StandardLayout::FieldId, MAXIMUM_MATERIAL_SLOTS> materialNames;
        Emergence::StandardLayout::FieldId implementationHandle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
