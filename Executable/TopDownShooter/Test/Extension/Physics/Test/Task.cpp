#include <thread>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/Simulation.hpp>
#include <Physics/Test/Task.hpp>

#include <Testing/Testing.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Emergence::Physics::Test
{
class Configurator final : public Celerity::TaskExecutorBase<Configurator>
{
public:
    Configurator (Celerity::TaskConstructor &_constructor, Container::Vector<ConfiguratorFrame> _frames) noexcept;

    void Execute ();

private:
    uint64_t currentFrameIndex = 0u;
    Container::Vector<ConfiguratorFrame> frames;
    Container::Vector<ConfiguratorFrame>::const_iterator framesIterator;

    Celerity::InsertLongTermQuery insertMaterial;
    Celerity::ModifyValueQuery modifyMaterialById;

    Celerity::InsertLongTermQuery insertTransform;
    Celerity::ModifyValueQuery modifyTransformById;

    Celerity::InsertLongTermQuery insertBody;
    Celerity::ModifyValueQuery modifyBodyById;

    Celerity::InsertLongTermQuery insertShape;
    Celerity::ModifyValueQuery modifyShapeByShapeId;
};

Configurator::Configurator (Celerity::TaskConstructor &_constructor,
                            Container::Vector<ConfiguratorFrame> _frames) noexcept
    : frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      insertMaterial (_constructor.MInsertLongTerm (DynamicsMaterial)),
      modifyMaterialById (_constructor.MModifyValue1F (DynamicsMaterial, id)),

      insertTransform (_constructor.MInsertLongTerm (Transform::Transform3dComponent)),
      modifyTransformById (_constructor.MModifyValue1F (Transform::Transform3dComponent, objectId)),

      insertBody (_constructor.MInsertLongTerm (RigidBodyComponent)),
      modifyBodyById (_constructor.MModifyValue1F (RigidBodyComponent, objectId)),

      insertShape (_constructor.MInsertLongTerm (CollisionShapeComponent)),
      modifyShapeByShapeId (_constructor.MModifyValue1F (CollisionShapeComponent, shapeId))
{
    _constructor.MakeDependencyOf (Simulation::Checkpoint::SIMULATION_STARTED);
}

void Configurator::Execute ()
{
    if (framesIterator == frames.end () || framesIterator->frameIndex < currentFrameIndex)
    {
        ++currentFrameIndex;
        return;
    }

    assert (framesIterator->frameIndex == currentFrameIndex);
    for (const ConfiguratorTask &task : framesIterator->tasks)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddDynamicsMaterial>)
                {
                    LOG ("Adding DynamicsMaterial with id \"", _task.id, "\".");
                    auto cursor = insertMaterial.Execute ();
                    auto *material = static_cast<DynamicsMaterial *> (++cursor);

                    material->id = _task.id;
                    material->dynamicFriction = _task.dynamicFriction;
                    material->staticFriction = _task.staticFriction;
                    material->enableFriction = _task.enableFriction;
                    material->restitution = _task.restitution;
                    material->density = _task.density;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveDynamicsMaterial>)
                {
                    LOG ("Removing DynamicsMaterial with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddTransform>)
                {
                    LOG ("Adding Transform3dComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertTransform.Execute ();
                    auto *transform = static_cast<Transform::Transform3dComponent *> (++cursor);
                    transform->SetObjectId (_task.objectId);
                    transform->SetLogicalLocalTransform (_task.transform);
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveTransform>)
                {
                    LOG ("Removing Transform3dComponent from object with id ", _task.objectId, ".");
                    auto cursor = modifyTransformById.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddRigidBody>)
                {
                    LOG ("Adding RigidBodyComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertBody.Execute ();
                    auto *body = static_cast<RigidBodyComponent *> (++cursor);

                    body->objectId = _task.objectId;
                    body->type = _task.type;
                    body->linearDamping = _task.linearDamping;
                    body->angularDamping = _task.angularDamping;

                    body->continuousCollisionDetection = _task.continuousCollisionDetection;
                    body->affectedByGravity = _task.affectedByGravity;
                    body->manipulatedOutsideOfSimulation = _task.manipulatedOutsideOfSimulation;
                    body->sendContactEvents = _task.sendContactEvents;
                    body->linearVelocity = _task.linearVelocity;
                    body->angularVelocity = _task.angularVelocity;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveRigidBody>)
                {
                    LOG ("Removing RigidBodyComponent from object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddCollisionShape>)
                {
                    LOG ("Adding CollisionShapeComponent with shape id ", _task.shapeId, " to object with id ",
                         _task.objectId, ".");

                    auto cursor = insertShape.Execute ();
                    auto *shape = static_cast<CollisionShapeComponent *> (++cursor);

                    shape->shapeId = _task.shapeId;
                    shape->objectId = _task.objectId;
                    shape->materialId = _task.materialId;
                    shape->geometry = _task.geometry;
                    shape->translation = _task.translation;
                    shape->rotation = _task.rotation;

                    shape->enabled = _task.enabled;
                    shape->trigger = _task.trigger;
                    shape->visibleToWorldQueries = _task.visibleToWorldQueries;
                    shape->collisionGroup = _task.collisionGroup;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveCollisionShape>)
                {
                    LOG ("Removing CollisionShapeComponent with shape id ", _task.shapeId, ".");
                    auto cursor = modifyShapeByShapeId.Execute (&_task.shapeId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
            },
            task);
    }

    ++currentFrameIndex;
    ++framesIterator;
}

class Validator : public Celerity::TaskExecutorBase<Validator>
{
public:
    Validator (Celerity::TaskConstructor &_constructor, Container::Vector<ValidatorFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    uint64_t currentFrameIndex = 0u;
    Container::Vector<ValidatorFrame> frames;
    Container::Vector<ValidatorFrame>::const_iterator framesIterator;

    Celerity::FetchValueQuery fetchBodyById;
    Celerity::FetchValueQuery fetchShapeByShapeId;

    Celerity::FetchSequenceQuery fetchContactFoundEvents;
    Celerity::FetchSequenceQuery fetchContactPersistsEvents;
    Celerity::FetchSequenceQuery fetchContactLostEvents;

    Celerity::FetchSequenceQuery fetchTriggerEnteredEvents;
    Celerity::FetchSequenceQuery fetchTriggerExitedEvents;
};

Validator::Validator (Celerity::TaskConstructor &_constructor, Container::Vector<ValidatorFrame> _frames) noexcept
    : frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      fetchBodyById (_constructor.MFetchValue1F (RigidBodyComponent, objectId)),
      fetchShapeByShapeId (_constructor.MFetchValue1F (CollisionShapeComponent, shapeId)),

      fetchContactFoundEvents (_constructor.MFetchSequence (ContactFoundEvent)),
      fetchContactPersistsEvents (_constructor.MFetchSequence (ContactPersistsEvent)),
      fetchContactLostEvents (_constructor.MFetchSequence (ContactLostEvent)),

      fetchTriggerEnteredEvents (_constructor.MFetchSequence (TriggerEnteredEvent)),
      fetchTriggerExitedEvents (_constructor.MFetchSequence (TriggerExitedEvent))
{
    _constructor.DependOn (Simulation::Checkpoint::SIMULATION_FINISHED);
}

void Validator::Execute () noexcept
{
    if (framesIterator == frames.end () || framesIterator->frameIndex < currentFrameIndex)
    {
        ++currentFrameIndex;
        return;
    }

    assert (framesIterator->frameIndex == currentFrameIndex);
    for (const ValidatorTask &task : framesIterator->tasks)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, ValidatorTasks::CheckRigidBodyExistence>)
                {
                    LOG ("Checking that rigid body ", _task.shouldExist ? "exists" : "doesn't exist",
                         " on object with id ", _task.objectId);

                    auto cursor = fetchBodyById.Execute (&_task.objectId);
                    const bool exists = *cursor;
                    CHECK_EQUAL (exists, _task.shouldExist);
                }
                else if constexpr (std::is_same_v<Type, ValidatorTasks::CheckCollisionShapeExistence>)
                {
                    LOG ("Checking that collision shape with id ", _task.shapeId, " ",
                         _task.shouldExist ? "exists" : "doesn't exist", ".");

                    auto cursor = fetchShapeByShapeId.Execute (&_task.shapeId);
                    const bool exists = *cursor;
                    CHECK_EQUAL (exists, _task.shouldExist);
                }
            },
            task);
    }

    ++currentFrameIndex;
    ++framesIterator;
}

void ExecuteScenario (Container::Vector<ConfiguratorFrame> _configuratorFrames,
                      Container::Vector<ValidatorFrame> _validatorFrames)
{
    using namespace Memory::Literals;

    const std::uint64_t frames = std::max (_configuratorFrames.back ().frameIndex, _validatorFrames.back ().frameIndex);
    Celerity::World world {"TestWorld"_us, Celerity::WorldConfiguration {{TEST_FIXED_FRAME_S}}};

    {
        Celerity::EventRegistrar registrar {&world};
        RegisterEvents (registrar);
        Transform::RegisterEvents (registrar);
    }

    Celerity::PipelineBuilder builder {&world};
    builder.Begin ("FixedUpdate"_us, Celerity::PipelineType::FIXED);

    builder.AddCheckpoint (Simulation::Checkpoint::SIMULATION_STARTED);
    builder.AddCheckpoint (Simulation::Checkpoint::SIMULATION_FINISHED);
    Simulation::AddToFixedUpdate (builder);

    builder.AddTask ("Configurator"_us).SetExecutor<Configurator> (std::move (_configuratorFrames));
    builder.AddTask ("Validator"_us).SetExecutor<Validator> (std::move (_validatorFrames));
    REQUIRE (builder.End (std::thread::hardware_concurrency ()));

    for (std::uint64_t frameIndex = 0u; frameIndex <= frames; ++frameIndex)
    {
        Celerity::WorldTestingUtility::RunFixedUpdateOnce (world);
    }
}
} // namespace Emergence::Physics::Test
