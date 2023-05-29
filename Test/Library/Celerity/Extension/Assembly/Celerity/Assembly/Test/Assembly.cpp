#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_PADDING_WARNING

#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Assembly/Test/Data.hpp>
#include <Celerity/Assembly/Test/Task.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;

namespace Emergence::Celerity::Test
{
static const FixedComponentA STATE_ROOT_A_PROTO {ASSEMBLY_ROOT_OBJECT_ID, 0u, 12u};
static const FixedComponentA STATE_ROOT_A_ADDED {1u, 0u, 12u};
static const FixedComponentA STATE_ROOT_A_ADDED_RECURSIVE {3u, 0u, 12u};
static const FixedComponentB STATE_ROOT_B_PROTO {ASSEMBLY_ROOT_OBJECT_ID, true, false, false, true};
static const FixedComponentB STATE_ROOT_B_ADDED {1u, true, false, false, true};
static const FixedComponentB STATE_ROOT_B_ADDED_RECURSIVE {3u, true, false, false, true};

static const FixedMultiComponent STATE_MULTI_FIRST_PROTO {ASSEMBLY_ROOT_OBJECT_ID, 10u, 0.0f, 1.0f};
static const FixedMultiComponent STATE_MULTI_FIRST_ADDED {1u, 0u, 0.0f, 1.0f};
static const FixedMultiComponent STATE_MULTI_SECOND_PROTO {ASSEMBLY_ROOT_OBJECT_ID, 20u, 1.0f, 0.0f};
static const FixedMultiComponent STATE_MULTI_SECOND_ADDED {1u, 1u, 1.0f, 0.0f};

static const VelocityFixedComponent STATE_ROOT_VELOCITY_PROTO {ASSEMBLY_ROOT_OBJECT_ID, {0.0f, 0.0f, 10.0f}};
static const VelocityFixedComponent STATE_ROOT_VELOCITY_ADDED_NO_ROTATION {1u, {0.0f, 0.0f, 10.0f}};
static const VelocityFixedComponent STATE_ROOT_VELOCITY_ADDED_ROTATED {1u, {10.0f, 0.0f, 0.0f}};

static Transform3dComponent CreateTransform3d (UniqueId _objectId, UniqueId _parentObjectId)
{
    Transform3dComponent component;
    component.SetObjectId (_objectId);
    component.SetParentObjectId (_parentObjectId);
    return component;
}

static const Transform3dComponent STATE_FIRST_LEVEL_CHILD_PROTO = CreateTransform3d (1u, ASSEMBLY_ROOT_OBJECT_ID);
static const Transform3dComponent STATE_FIRST_LEVEL_CHILD_ADDED = CreateTransform3d (2u, 1u);

static const Transform3dComponent STATE_SECOND_LEVEL_CHILD_PROTO = CreateTransform3d (2u, 1u);
static const Transform3dComponent STATE_SECOND_LEVEL_CHILD_ADDED = CreateTransform3d (3u, 2u);

static const FixedComponentA STATE_SECOND_LEVEL_A_PROTO {2u, 12u, 0u};
static const FixedComponentA STATE_SECOND_LEVEL_A_ADDED {3u, 12u, 0u};

static const NormalVisualComponent STATE_ROOT_VISUAL_PROTO {ASSEMBLY_ROOT_OBJECT_ID, 10u, "Knight"_us, "GreyKnight"_us};
static const NormalVisualComponent STATE_ROOT_VISUAL_ADDED {1u, 0u, "Knight"_us, "GreyKnight"_us};

static const NormalVisualComponent STATE_ROOT_VISUAL_PROTO_NO_REF {ASSEMBLY_ROOT_OBJECT_ID, INVALID_UNIQUE_ID,
                                                                   "Knight"_us, "GreyKnight"_us};
static const NormalVisualComponent STATE_ROOT_VISUAL_ADDED_NO_REF {1u, INVALID_UNIQUE_ID, "Knight"_us, "GreyKnight"_us};

static const PrototypeComponent STATE_PROTOTYPE_AB {1u, "AB"_us, true, false, false};
static const PrototypeComponent STATE_PROTOTYPE_PROTOTYPE_AB {1u, "PrototypeAB"_us, true, false, false};

void FixedAssemblyTest (Container::Vector<ConfiguratorTask> _configuratorTasks,
                        Container::Vector<ValidatorTask> _validatorTasks)
{
    using namespace Memory::Literals;

    World world {"TestWorld"_us, WorldConfiguration {}};

    {
        EventRegistrar registrar {&world};
        RegisterAssemblyEvents (registrar);
    }

    PipelineBuilder builder {world.GetRootView ()};
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);

    Assembly::AddToFixedUpdate (builder, GetAssemblerCustomKeys (), GetFixedAssemblerTypes (), 16000000u);
    AddConfiguratorAndValidator (builder, std::move (_configuratorTasks), std::move (_validatorTasks));
    REQUIRE (builder.End ());

    WorldTestingUtility::RunFixedUpdateOnce (world);
}

void CombinedAssemblyTest (Container::Vector<ConfiguratorTask> _fixedConfiguratorTasks,
                           Container::Vector<ValidatorTask> _fixedValidatorTasks,
                           Container::Vector<ConfiguratorTask> _normalConfiguratorTasks,
                           Container::Vector<ValidatorTask> _normalValidatorTasks)
{
    using namespace Memory::Literals;

    World world {"TestWorld"_us, WorldConfiguration {}};

    {
        EventRegistrar registrar {&world};
        RegisterAssemblyEvents (registrar);
    }

    PipelineBuilder builder {world.GetRootView ()};
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);
    Assembly::AddToFixedUpdate (builder, GetAssemblerCustomKeys (), GetFixedAssemblerTypes (), 16000000u);
    AddConfiguratorAndValidator (builder, std::move (_fixedConfiguratorTasks), std::move (_fixedValidatorTasks));
    REQUIRE (builder.End ());

    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);
    builder.AddCheckpoint (TransformVisualSync::Checkpoint::STARTED);
    Assembly::AddToNormalUpdate (builder, GetAssemblerCustomKeys (), GetNormalAssemblerTypes (), 16000000u);
    AddConfiguratorAndValidator (builder, std::move (_normalConfiguratorTasks), std::move (_normalValidatorTasks));
    REQUIRE (builder.End ());

    WorldTestingUtility::RunFixedUpdateOnce (world);
    WorldTestingUtility::RunNormalUpdateOnce (world, 16000000u);
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Math;

BEGIN_SUITE (AssemblyLogic)

TEST_CASE (OneComponent)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"A"_us, {{FixedComponentA::Reflect ().mapping, &STATE_ROOT_A_PROTO}}},
            SpawnPrototype {"A"_us},
        },
        {
            CheckComponent {1u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_ROOT_A_ADDED},
            CheckComponent {1u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId, nullptr},
        });
}

TEST_CASE (TwoComponents)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"AB"_us,
                                   {{FixedComponentA::Reflect ().mapping, &STATE_ROOT_A_PROTO},
                                    {FixedComponentB::Reflect ().mapping, &STATE_ROOT_B_PROTO}}},
            SpawnPrototype {"AB"_us},
        },
        {
            CheckComponent {1u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_ROOT_A_ADDED},
            CheckComponent {1u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_ROOT_B_ADDED},
        });
}

TEST_CASE (UnknownDescriptor)
{
    FixedAssemblyTest (
        {
            SpawnPrototype {"Unknown"_us},
        },
        {
            CheckComponent {1u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId, nullptr},
            CheckComponent {1u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId, nullptr},
        });
}

TEST_CASE (MultiComponents)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"Multi"_us,
                                   {{FixedMultiComponent::Reflect ().mapping, &STATE_MULTI_FIRST_PROTO},
                                    {FixedMultiComponent::Reflect ().mapping, &STATE_MULTI_SECOND_PROTO}}},
            SpawnPrototype {"Multi"_us},
        },
        {
            CheckComponent {0u, FixedMultiComponent::Reflect ().mapping, FixedMultiComponent::Reflect ().instanceId,
                            &STATE_MULTI_FIRST_ADDED},
            CheckComponent {1u, FixedMultiComponent::Reflect ().mapping, FixedMultiComponent::Reflect ().instanceId,
                            &STATE_MULTI_SECOND_ADDED},
        });
}

TEST_CASE (VectorIdentityRotation)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"WithVelocity"_us,
                                   {{VelocityFixedComponent::Reflect ().mapping, &STATE_ROOT_VELOCITY_PROTO}}},
            SpawnPrototype {"WithVelocity"_us},
        },
        {
            CheckVector3f {1u, VelocityFixedComponent::Reflect ().mapping, VelocityFixedComponent::Reflect ().objectId,
                           VelocityFixedComponent::Reflect ().globalVelocity,
                           STATE_ROOT_VELOCITY_ADDED_NO_ROTATION.globalVelocity},
        });
}

TEST_CASE (VectorRotated)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"WithVelocity"_us,
                                   {{VelocityFixedComponent::Reflect ().mapping, &STATE_ROOT_VELOCITY_PROTO}}},
            SpawnPrototype {"WithVelocity"_us,
                            {Vector3f::ONE, Emergence::Math::Quaternion {{0.0f, PI * 0.5f, 0.0f}}, Vector3f::ONE}},
        },
        {
            CheckVector3f {1u, VelocityFixedComponent::Reflect ().mapping, VelocityFixedComponent::Reflect ().objectId,
                           VelocityFixedComponent::Reflect ().globalVelocity,
                           STATE_ROOT_VELOCITY_ADDED_ROTATED.globalVelocity},
        });
}

TEST_CASE (Hierarchy)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"Hierarchy"_us,
                                   {{Transform3dComponent::Reflect ().mapping, &STATE_FIRST_LEVEL_CHILD_PROTO},
                                    {Transform3dComponent::Reflect ().mapping, &STATE_SECOND_LEVEL_CHILD_PROTO},
                                    {FixedComponentA::Reflect ().mapping, &STATE_SECOND_LEVEL_A_PROTO}}},
            SpawnPrototype {"Hierarchy"_us},
        },
        {
            CheckComponent {2u, Transform3dComponent::Reflect ().mapping, Transform3dComponent::Reflect ().objectId,
                            // We only check the begging of an object, because checking
                            // whole transform usually fails due to padding bytes.
                            &STATE_FIRST_LEVEL_CHILD_ADDED, sizeof (UniqueId) * 5u},
            CheckComponent {3u, Transform3dComponent::Reflect ().mapping, Transform3dComponent::Reflect ().objectId,
                            &STATE_SECOND_LEVEL_CHILD_ADDED, sizeof (UniqueId) * 5u},
            CheckComponent {3u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_SECOND_LEVEL_A_ADDED},
        });
}

TEST_CASE (RecursiveImmediateAssembly)
{
    FixedAssemblyTest (
        {
            AddAssemblyDescriptor {"AB"_us,
                                   {{FixedComponentA::Reflect ().mapping, &STATE_ROOT_A_PROTO},
                                    {FixedComponentB::Reflect ().mapping, &STATE_ROOT_B_PROTO}}},
            AddAssemblyDescriptor {
                "PrototypeAB"_us,
                {{Transform3dComponent::Reflect ().mapping, &STATE_FIRST_LEVEL_CHILD_PROTO},
                 {PrototypeComponent::Reflect ().mapping, &STATE_PROTOTYPE_AB}},
            },
            AddAssemblyDescriptor {
                "PrototypeOfPrototype"_us,
                {{Transform3dComponent::Reflect ().mapping, &STATE_FIRST_LEVEL_CHILD_PROTO},
                 {PrototypeComponent::Reflect ().mapping, &STATE_PROTOTYPE_PROTOTYPE_AB}},
            },
            SpawnPrototype {"PrototypeOfPrototype"_us},
        },
        {
            CheckComponent {3u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_ROOT_A_ADDED_RECURSIVE},
            CheckComponent {3u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_ROOT_B_ADDED_RECURSIVE},
        });
}

TEST_CASE (InvalidReferenceIgnored)
{
    CombinedAssemblyTest (
        {
            AddAssemblyDescriptor {"V"_us,
                                   {{NormalVisualComponent::Reflect ().mapping, &STATE_ROOT_VISUAL_PROTO_NO_REF}}},
            SpawnPrototype {"V"_us},
        },
        {}, {},
        {
            CheckComponent {1u, NormalVisualComponent::Reflect ().mapping, NormalVisualComponent::Reflect ().objectId,
                            &STATE_ROOT_VISUAL_ADDED_NO_REF},
        });
}

TEST_CASE (FixedAndNormalCombined)
{
    CombinedAssemblyTest (
        {
            AddAssemblyDescriptor {"ABMV"_us,
                                   {{FixedComponentA::Reflect ().mapping, &STATE_ROOT_A_PROTO},
                                    {FixedComponentB::Reflect ().mapping, &STATE_ROOT_B_PROTO},
                                    {FixedMultiComponent::Reflect ().mapping, &STATE_MULTI_FIRST_PROTO},
                                    {NormalVisualComponent::Reflect ().mapping, &STATE_ROOT_VISUAL_PROTO}}},
            SpawnPrototype {"ABMV"_us},
        },
        {
            CheckComponent {1u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_ROOT_A_ADDED},
            CheckComponent {1u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_ROOT_B_ADDED},
            CheckComponent {1u, FixedMultiComponent::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_MULTI_FIRST_ADDED},
            CheckComponent {1u, NormalVisualComponent::Reflect ().mapping, NormalVisualComponent::Reflect ().objectId,
                            nullptr},
        },
        {},
        {
            CheckComponent {1u, FixedComponentA::Reflect ().mapping, FixedComponentA::Reflect ().objectId,
                            &STATE_ROOT_A_ADDED},
            CheckComponent {1u, FixedComponentB::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_ROOT_B_ADDED},
            CheckComponent {1u, FixedMultiComponent::Reflect ().mapping, FixedComponentB::Reflect ().objectId,
                            &STATE_MULTI_FIRST_ADDED},
            CheckComponent {1u, NormalVisualComponent::Reflect ().mapping, NormalVisualComponent::Reflect ().objectId,
                            &STATE_ROOT_VISUAL_ADDED},
        });
}

END_SUITE

END_MUTING_WARNINGS
