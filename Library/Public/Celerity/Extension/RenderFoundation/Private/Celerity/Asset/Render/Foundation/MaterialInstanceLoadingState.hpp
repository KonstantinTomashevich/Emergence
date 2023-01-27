#pragma once

#include <Celerity/Render/Foundation/Material.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/MuteWarnings.hpp>

namespace Emergence::Celerity
{
struct MaterialInstanceLoadingState final
{
    Memory::UniqueString assetId;
    Memory::UniqueString parentId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId parentId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
