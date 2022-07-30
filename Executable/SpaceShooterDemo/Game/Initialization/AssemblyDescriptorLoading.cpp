#include <filesystem>
#include <fstream>

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

#include <Initialization/AssemblyDescriptorLoading.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Log/Log.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/StaticModelComponent.hpp>

#include <Serialization/Yaml.hpp>

namespace AssemblyDescriptorLoading
{
class AssemblyDescriptorLoader final : public Emergence::Celerity::TaskExecutorBase<AssemblyDescriptorLoader>
{
public:
    AssemblyDescriptorLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::InsertLongTermQuery insertAssemblyDescriptor;
};

AssemblyDescriptorLoader::AssemblyDescriptorLoader (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : insertAssemblyDescriptor (INSERT_LONG_TERM (Emergence::Celerity::AssemblyDescriptor))
{
}

void AssemblyDescriptorLoader::Execute () noexcept
{
    static const char *materialsPath = "../GameAssets/Objects/";
    auto cursor = insertAssemblyDescriptor.Execute ();

    Emergence::Serialization::Yaml::BundleDeserializationContext context;
    context.RegisterType (Emergence::Celerity::CollisionShapeComponent::Reflect ().mapping);
    context.RegisterType (Emergence::Celerity::RigidBodyComponent::Reflect ().mapping);
    context.RegisterType (Emergence::Celerity::Transform3dComponent::Reflect ().mapping);
    context.RegisterType (ControllableComponent::Reflect ().mapping);
    context.RegisterType (DamageDealerComponent::Reflect ().mapping);
    context.RegisterType (MortalComponent::Reflect ().mapping);
    context.RegisterType (MovementComponent::Reflect ().mapping);
    context.RegisterType (ShooterComponent::Reflect ().mapping);
    context.RegisterType (InputListenerComponent::Reflect ().mapping);
    context.RegisterType (StaticModelComponent::Reflect ().mapping);
    context.RegisterType (ParticleEffectComponent::Reflect ().mapping);

    for (const auto &entry : std::filesystem::directory_iterator (materialsPath))
    {
        const auto &path = entry.path ();
        if (entry.is_regular_file () && path.extension ().string () == ".yaml" && !path.stem ().empty ())
        {
            EMERGENCE_LOG (DEBUG, "AssemblyDescriptorLoading: Loading descriptor \"", path.stem ().string ().c_str (),
                           "\" from file \"", path.string ().c_str (), "\"...");
            std::ifstream input {entry};

            if (!input)
            {
                EMERGENCE_LOG (ERROR, "AssemblyDescriptorLoading: Unable to open file \"",
                               entry.path ().string ().c_str (), "\"!");
            }

            auto *descriptor = static_cast<Emergence::Celerity::AssemblyDescriptor *> (++cursor);
            descriptor->id = Emergence::Memory::UniqueString {path.stem ().string ().c_str ()};

            if (!Emergence::Serialization::Yaml::DeserializePatchBundle (input, descriptor->components, context))
            {
                EMERGENCE_LOG (ERROR, "AssemblyDescriptorLoading: Failed to load components bundle from \"",
                               entry.path ().string ().c_str (), "\"!");
            }
        }
    }
}

using namespace Emergence::Memory::Literals;

void AddToInitializationPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("AssemblyDescriptorLoader"_us).SetExecutor<AssemblyDescriptorLoader> ();
}
} // namespace AssemblyDescriptorLoading
