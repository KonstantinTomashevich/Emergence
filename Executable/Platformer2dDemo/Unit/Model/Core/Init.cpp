#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Nexus/Nexus.hpp>

#include <Configuration/ResourceProviderTypes.hpp>

#include <Core/NexusUserContext.hpp>

#include <Resource/Provider/Helpers.hpp>

Platformer2dDemoModelApi extern "C" __cdecl void InitModel (Emergence::Celerity::Nexus *_nexus)
{
    auto *context = static_cast<NexusUserContext *> (_nexus->GetUserContext ());
    context->resourceProvider.reset ();

    context->resourceProvider.emplace (&context->virtualFileSystem, GetResourceTypesRegistry (),
                                       GetPatchableTypesRegistry ());

    Emergence::Resource::Provider::AddMountedDirectoriesAsSources (context->resourceProvider.value (),
                                                                   context->resourcesRoot, context->resourcesMount);
}
