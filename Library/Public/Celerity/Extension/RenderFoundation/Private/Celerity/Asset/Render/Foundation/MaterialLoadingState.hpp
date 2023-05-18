#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class MaterialLoadingSharedState final : public LoadingSharedState<MaterialLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialLoading";

    MaterialAsset asset;
};

struct MaterialLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<MaterialLoadingSharedState> sharedState {new MaterialLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
