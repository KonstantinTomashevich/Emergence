#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class MaterialInstanceLoadingSharedState final : public LoadingSharedState<MaterialInstanceLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialInstanceLoading";

    MaterialInstanceAsset asset;
};

struct MaterialInstanceLoadingState final
{
    Memory::UniqueString assetId;

    Memory::UniqueString parentId;

    Handling::Handle<MaterialInstanceLoadingSharedState> sharedState {new MaterialInstanceLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId parentId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
