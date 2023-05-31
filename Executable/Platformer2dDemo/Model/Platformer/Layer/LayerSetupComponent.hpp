#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct LayerSetupComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    std::uint16_t layer = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId layer;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
