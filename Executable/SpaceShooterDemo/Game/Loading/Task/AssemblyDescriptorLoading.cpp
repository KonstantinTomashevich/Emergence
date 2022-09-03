#include <filesystem>
#include <fstream>

#include <Asset/Object/LibraryLoader.hpp>

#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Physics/CollisionShapeComponent.hpp>
#include <Celerity/Physics/RigidBodyComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>

#include <Gameplay/ControllableComponent.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/MovementComponent.hpp>
#include <Gameplay/ShooterComponent.hpp>

#include <Loading/Model/AssemblyDescriptorLoadingSingleton.hpp>
#include <Loading/Task/AssemblyDescriptorLoading.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/StaticModelComponent.hpp>

namespace AssemblyDescriptorLoading
{
const Emergence::Memory::UniqueString Checkpoint::STEP_FINISHED {"AssemblyDescriptorLoadingStepFinished"};

class AssemblyDescriptorLoader final : public Emergence::Celerity::TaskExecutorBase<AssemblyDescriptorLoader>
{
public:
    AssemblyDescriptorLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    static Emergence::Asset::Object::TypeManifest PrepareTypeManifest () noexcept;

    Emergence::Asset::Object::LibraryLoader libraryLoader;
    Emergence::Celerity::ModifySingletonQuery modifyState;
    Emergence::Celerity::InsertLongTermQuery insertAssemblyDescriptor;
};

AssemblyDescriptorLoader::AssemblyDescriptorLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : libraryLoader (PrepareTypeManifest ()),
      modifyState (MODIFY_SINGLETON (AssemblyDescriptorLoadingSingleton)),
      insertAssemblyDescriptor (INSERT_LONG_TERM (Emergence::Celerity::AssemblyDescriptor))
{
    _constructor.MakeDependencyOf (Checkpoint::STEP_FINISHED);
}

void AssemblyDescriptorLoader::Execute () noexcept
{
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<AssemblyDescriptorLoadingSingleton *> (*stateCursor);

    if (state->finished)
    {
        return;
    }

    if (!state->started)
    {
        static const char *objectsPath = "../GameAssets/Objects/";
        libraryLoader.Begin ({{objectsPath, Emergence::Memory::UniqueString {}}});
        state->started = true;
    }

    if (libraryLoader.IsLoading ())
    {
        return;
    }

    Emergence::Asset::Object::Library library = libraryLoader.End ();
    auto cursor = insertAssemblyDescriptor.Execute ();

    for (const auto &[objectName, objectData] : library.GetRegisteredObjectMap ())
    {
        if (objectData.loadedAsDependency)
        {
            continue;
        }

        auto *descriptor = static_cast<Emergence::Celerity::AssemblyDescriptor *> (++cursor);
        descriptor->id = objectName;
        descriptor->components.reserve (objectData.body.fullChangelist.size ());

        for (const Emergence::StandardLayout::Patch &patch : objectData.body.fullChangelist)
        {
            descriptor->components.emplace_back (patch);
        }
    }

    state->finished = true;
}

Emergence::Asset::Object::TypeManifest AssemblyDescriptorLoader::PrepareTypeManifest () noexcept
{
    Emergence::Asset::Object::TypeManifest typeManifest;
    typeManifest.Register (ControllableComponent::Reflect ().mapping, {ControllableComponent::Reflect ().objectId});
    typeManifest.Register (DamageDealerComponent::Reflect ().mapping, {DamageDealerComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::CollisionShapeComponent::Reflect ().mapping,
                           {Emergence::Celerity::CollisionShapeComponent::Reflect ().shapeId});

    typeManifest.Register (Emergence::Celerity::RigidBodyComponent::Reflect ().mapping,
                           {Emergence::Celerity::RigidBodyComponent::Reflect ().objectId});

    typeManifest.Register (Emergence::Celerity::Transform3dComponent::Reflect ().mapping,
                           {Emergence::Celerity::Transform3dComponent::Reflect ().objectId});

    typeManifest.Register (InputListenerComponent::Reflect ().mapping, {InputListenerComponent::Reflect ().objectId});

    typeManifest.Register (MortalComponent::Reflect ().mapping, {MortalComponent::Reflect ().objectId});
    typeManifest.Register (MovementComponent::Reflect ().mapping, {MovementComponent::Reflect ().objectId});

    typeManifest.Register (ParticleEffectComponent::Reflect ().mapping, {ParticleEffectComponent::Reflect ().effectId});

    typeManifest.Register (ShooterComponent::Reflect ().mapping, {ShooterComponent::Reflect ().objectId});
    typeManifest.Register (StaticModelComponent::Reflect ().mapping, {StaticModelComponent::Reflect ().modelId});
    return typeManifest;
}

using namespace Emergence::Memory::Literals;

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STEP_FINISHED);
    _pipelineBuilder.AddTask ("AssemblyDescriptorLoader"_us).SetExecutor<AssemblyDescriptorLoader> ();
}
} // namespace AssemblyDescriptorLoading
