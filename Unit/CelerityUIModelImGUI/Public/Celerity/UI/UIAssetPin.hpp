#pragma once

#include <CelerityUIModelImGUIApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct CelerityUIModelImGUIApi UIAssetPin final
{
    Memory::UniqueString materialId;

    struct CelerityUIModelImGUIApi Reflection final
    {
        StandardLayout::FieldId materialId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
