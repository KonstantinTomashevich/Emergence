#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Nexus/Nexus.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Render/2d/AssetUsage.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/Foundation/AssetUsage.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/Events.hpp>

#include <Configuration/AssetUsage.hpp>

#include <Core/NexusUserContext.hpp>

#include <Platformer/Events.hpp>

extern "C" Platformer2dDemoModelApi void __cdecl RegisterAllEvents (Emergence::Celerity::Nexus *_nexus,
                                                            Emergence::Celerity::EventRegistrar &_registrar)
{
    auto *context = static_cast<NexusUserContext *> (_nexus->GetUserContext ());
    auto &assetReferenceBindingList =
        context->assetReferenceBindingList.Emplace<Emergence::Celerity::AssetReferenceBindingList> (
            Emergence::Celerity::GetAssetBindingAllocationGroup ());
    auto &assetReferenceBindingEventMap =
        context->assetReferenceBindingEventMap.Emplace<Emergence::Celerity::AssetReferenceBindingEventMap> ();

    Emergence::Celerity::GetRender2dAssetUsage (assetReferenceBindingList);
    Emergence::Celerity::GetRenderFoundationAssetUsage (assetReferenceBindingList);
    Emergence::Celerity::GetUIAssetUsage (assetReferenceBindingList);
    GetGameAssetUsage (assetReferenceBindingList);

    Emergence::Celerity::RegisterAssemblyEvents (_registrar);
    assetReferenceBindingEventMap = Emergence::Celerity::RegisterAssetEvents (_registrar, assetReferenceBindingList);
    Emergence::Celerity::RegisterPhysicsEvents (_registrar);
    Emergence::Celerity::RegisterRender2dEvents (_registrar);
    Emergence::Celerity::RegisterRenderFoundationEvents (_registrar);
    Emergence::Celerity::RegisterTransform2dEvents (_registrar);
    Emergence::Celerity::RegisterTransformCommonEvents (_registrar);
    Emergence::Celerity::RegisterUIEvents (_registrar);
    RegisterPlatformerEvents (_registrar);
}
