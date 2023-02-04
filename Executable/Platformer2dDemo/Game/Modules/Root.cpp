#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/Events.hpp>

#include <Configuration/Paths.hpp>

#include <Modules/Root.hpp>

#include <Root/LevelSelectionSingleton.hpp>
#include <Root/LevelsConfigurationSingleton.hpp>

namespace Modules::Root
{
using namespace Emergence::Memory::Literals;

static Emergence::Celerity::AssetReferenceBindingList GetAssetReferenceBindingList ()
{
    Emergence::Celerity::AssetReferenceBindingList binding {Emergence::Celerity::GetAssetBindingAllocationGroup ()};
    Emergence::Celerity::GetRender2dAssetUsage (binding);
    Emergence::Celerity::GetRenderFoundationAssetUsage (binding);
    Emergence::Celerity::GetUIAssetUsage (binding);
    return binding;
}

Emergence::Celerity::WorldViewConfig GetViewConfig () noexcept
{
    static Emergence::Celerity::WorldViewConfig rootViewConfig = [] ()
    {
        Emergence::Celerity::WorldViewConfig config;
        config.enforcedTypes.emplace (LevelsConfigurationSingleton::Reflect ().mapping);
        config.enforcedTypes.emplace (LevelSelectionSingleton::Reflect ().mapping);
        return config;
    }();

    return rootViewConfig;
}

void Initializer (GameState & /*unused*/,
                  Emergence::Celerity::World &_world,
                  Emergence::Celerity::WorldView &_rootView) noexcept
{
    Emergence::Celerity::AssetReferenceBindingList assetReferenceBindingList = GetAssetReferenceBindingList ();
    Emergence::Celerity::AssetReferenceBindingEventMap assetReferenceBindingEventMap;

    {
        Emergence::Celerity::EventRegistrar registrar {&_world};
        Emergence::Celerity::RegisterAssemblyEvents (registrar);
        assetReferenceBindingEventMap = Emergence::Celerity::RegisterAssetEvents (registrar, assetReferenceBindingList);
        Emergence::Celerity::RegisterRender2dEvents (registrar);
        Emergence::Celerity::RegisterRenderFoundationEvents (registrar);
        Emergence::Celerity::RegisterTransform2dEvents (registrar);
        Emergence::Celerity::RegisterTransformCommonEvents (registrar);
        Emergence::Celerity::RegisterUIEvents (registrar);
    }

    Emergence::Celerity::PipelineBuilder pipelineBuilder {&_rootView};
    constexpr uint64_t MAX_LOADING_TIME_NS = 10000000;

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    Emergence::Celerity::AssetManagement::AddToNormalUpdate (pipelineBuilder, assetReferenceBindingList,
                                                             assetReferenceBindingEventMap);
    Emergence::Celerity::FontManagement::AddToNormalUpdate (pipelineBuilder, GetFontPaths (), MAX_LOADING_TIME_NS,
                                                            assetReferenceBindingEventMap);
    Emergence::Celerity::MaterialInstanceManagement::AddToNormalUpdate (
        pipelineBuilder, GetMaterialInstancePaths (), MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::MaterialManagement::AddToNormalUpdate (
        pipelineBuilder, GetMaterialPaths (), GetShadersPaths (), MAX_LOADING_TIME_NS, assetReferenceBindingEventMap);
    Emergence::Celerity::TextureManagement::AddToNormalUpdate (pipelineBuilder, GetTexturePaths (), MAX_LOADING_TIME_NS,
                                                               assetReferenceBindingEventMap);
    [[maybe_unused]] const bool normalPipelineRegistered = pipelineBuilder.End ();
    EMERGENCE_ASSERT (normalPipelineRegistered);
}
} // namespace Modules::Root
