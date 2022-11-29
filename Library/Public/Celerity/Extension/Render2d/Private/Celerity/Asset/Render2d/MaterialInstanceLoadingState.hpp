#pragma once

#include <Celerity/Render2d/Material.hpp>
#include <Celerity/Render2d/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

#include <SyntaxSugar/MuteWarnings.hpp>

namespace Emergence::Celerity
{
struct MaterialInstanceLoadingState final
{
    Memory::UniqueString assetId;
    UniqueId assetUserId;
    Memory::UniqueString parentId;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId parentId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
