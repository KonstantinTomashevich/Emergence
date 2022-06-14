#include <thread>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/Simulation.hpp>
#include <Physics/Test/Task.hpp>

#include <Testing/Testing.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

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

    Celerity::ModifySingletonQuery modifyPhysicsWorld;

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

      modifyPhysicsWorld (_constructor.MModifySingleton (PhysicsWorldSingleton)),

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
    if (currentFrameIndex == 0u)
    {
        // Fill test collision mask during first frame. For simplicity, group X collides only with group X objects.
        auto worldCursor = modifyPhysicsWorld.Execute ();
        auto *physicsWorld = static_cast<PhysicsWorldSingleton *> (*worldCursor);

        for (std::size_t group = 0u; group < physicsWorld->collisionMasks.size (); ++group)
        {
            physicsWorld->collisionMasks[group] = 1u << group;
        }
    }

    if (framesIterator == frames.end () || framesIterator->frameIndex > currentFrameIndex)
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

#define FILL_MATERIAL                                                                                                  \
    material->id = _task.id;                                                                                           \
    material->dynamicFriction = _task.dynamicFriction;                                                                 \
    material->staticFriction = _task.staticFriction;                                                                   \
    material->enableFriction = _task.enableFriction;                                                                   \
    material->restitution = _task.restitution;                                                                         \
    material->density = _task.density

                    FILL_MATERIAL;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateDynamicsMaterial>)
                {
                    LOG ("Updating DynamicsMaterial with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    auto *material = static_cast<DynamicsMaterial *> (*cursor);
                    REQUIRE (material);
                    FILL_MATERIAL;
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
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateTransform>)
                {
                    LOG ("Updating Transform3dComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyTransformById.Execute (&_task.objectId);
                    auto *transform = static_cast<Transform::Transform3dComponent *> (*cursor);
                    REQUIRE (transform);
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

#define FILL_BODY                                                                                                      \
    body->objectId = _task.objectId;                                                                                   \
    body->type = _task.type;                                                                                           \
    body->linearDamping = _task.linearDamping;                                                                         \
    body->angularDamping = _task.angularDamping;                                                                       \
                                                                                                                       \
    body->continuousCollisionDetection = _task.continuousCollisionDetection;                                           \
    body->affectedByGravity = _task.affectedByGravity;                                                                 \
    body->manipulatedOutsideOfSimulation = _task.manipulatedOutsideOfSimulation;                                       \
    body->linearVelocity = _task.linearVelocity;                                                                       \
    body->angularVelocity = _task.angularVelocity;                                                                     \
    body->additiveLinearImpulse = _task.additiveLinearImpulse;                                                         \
    body->additiveAngularImpulse = _task.additiveAngularImpulse;                                                       \
    body->lockFlags = _task.lockFlags

                    FILL_BODY;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateRigidBody>)
                {
                    LOG ("Update RigidBodyComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    auto *body = static_cast<RigidBodyComponent *> (*cursor);
                    REQUIRE (body);
                    FILL_BODY;
#undef FILL_BODY
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

#define FILL_SHAPE                                                                                                     \
    shape->shapeId = _task.shapeId;                                                                                    \
    shape->objectId = _task.objectId;                                                                                  \
    shape->materialId = _task.materialId;                                                                              \
    shape->geometry = _task.geometry;                                                                                  \
    shape->translation = _task.translation;                                                                            \
    shape->rotation = _task.rotation;                                                                                  \
                                                                                                                       \
    shape->enabled = _task.enabled;                                                                                    \
    shape->trigger = _task.trigger;                                                                                    \
    shape->visibleToWorldQueries = _task.visibleToWorldQueries;                                                        \
    shape->sendContactEvents = _task.sendContactEvents;                                                                \
    shape->collisionGroup = _task.collisionGroup

                    FILL_SHAPE;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateCollisionShape>)
                {
                    LOG ("Update CollisionShapeComponent with shape id ", _task.shapeId, " on object with id ",
                         _task.objectId, ".");

                    auto cursor = modifyShapeByShapeId.Execute (&_task.shapeId);
                    auto *shape = static_cast<CollisionShapeComponent *> (*cursor);
                    REQUIRE (shape);
                    REQUIRE (shape->objectId == _task.objectId);
                    FILL_SHAPE;
#undef FILL_SHAPE
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

    Celerity::FetchValueQuery fetchTransformById;
    Transform::Transform3dWorldAccessor transformWorldAccessor;

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

      fetchTransformById (_constructor.MFetchValue1F (Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

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
    if (framesIterator == frames.end () || framesIterator->frameIndex > currentFrameIndex)
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
                else if constexpr (std::is_same_v<Type, ValidatorTasks::CheckObjectTransform>)
                {
                    LOG ("Checking that transform of an object ", _task.objectId, " is equal to {{",
                         _task.transform.translation.x, ", ", _task.transform.translation.y, ", ",
                         _task.transform.translation.z, "}, {", _task.transform.rotation.x, ", ",
                         _task.transform.rotation.y, ", ", _task.transform.rotation.z, ", ", _task.transform.rotation.w,
                         "}, {", _task.transform.scale.x, ", ", _task.transform.scale.y, ", ", _task.transform.scale.z,
                         "}.");

                    auto cursor = fetchTransformById.Execute (&_task.objectId);
                    const auto *transform = static_cast<const Transform::Transform3dComponent *> (*cursor);
                    REQUIRE (transform);

                    const Math::Transform3d &worldTransform =
                        transform->GetLogicalWorldTransform (transformWorldAccessor);

                    // We use custom equality check, because we need very high custom epsilon.
                    // This epsilon is needed because it is difficult to receive integer results with 60 FPS delta.
                    auto nearlyEqual = [] (float _x, float _y, float _epsilon)
                    {
                        const float difference = _x - _y;
                        return difference > -_epsilon && difference < _epsilon;
                    };

                    CHECK (nearlyEqual (worldTransform.translation.x, _task.transform.translation.x, 0.05f));
                    CHECK (nearlyEqual (worldTransform.translation.y, _task.transform.translation.y, 0.05f));
                    CHECK (nearlyEqual (worldTransform.translation.z, _task.transform.translation.z, 0.05f));

                    CHECK (nearlyEqual (worldTransform.rotation.x, _task.transform.rotation.x, 0.001f));
                    CHECK (nearlyEqual (worldTransform.rotation.y, _task.transform.rotation.y, 0.001f));
                    CHECK (nearlyEqual (worldTransform.rotation.z, _task.transform.rotation.z, 0.001f));
                    CHECK (nearlyEqual (worldTransform.rotation.w, _task.transform.rotation.w, 0.001f));

                    CHECK (nearlyEqual (worldTransform.scale.x, _task.transform.scale.x, 0.00001f));
                    CHECK (nearlyEqual (worldTransform.scale.y, _task.transform.scale.y, 0.00001f));
                    CHECK (nearlyEqual (worldTransform.scale.z, _task.transform.scale.z, 0.00001f));
                }
                else if constexpr (std::is_same_v<Type, ValidatorTasks::CheckEvents>)
                {
                    auto checkEvents = [] (Celerity::FetchSequenceQuery &_query, const auto &_expected)
                    {
                        using VectorType = std::decay_t<decltype (_expected)>;
                        using ValueType = typename VectorType::value_type;
                        VectorType found;

                        for (auto cursor = _query.Execute ();
                             const auto *event = static_cast<const ValueType *> (*cursor); ++cursor)
                        {
                            found.emplace_back (*event);
                        }

                        // For simplicity, we assume that every event is unique.
                        CHECK_EQUAL (found.size (), _expected.size ());

                        for (const ValueType &event : _expected)
                        {
                            CHECK (std::find (found.begin (), found.end (), event) != found.end ());
                        }
                    };

                    LOG ("Checking contact found events...");
                    checkEvents (fetchContactFoundEvents, _task.contactFound);

                    LOG ("Checking contact persists events...");
                    checkEvents (fetchContactPersistsEvents, _task.contactPersists);

                    LOG ("Checking contact lost events...");
                    checkEvents (fetchContactLostEvents, _task.contactLost);

                    LOG ("Checking trigger entered events...");
                    checkEvents (fetchTriggerEnteredEvents, _task.triggerEntered);

                    LOG ("Checking trigger exited events...");
                    checkEvents (fetchTriggerExitedEvents, _task.triggerExited);
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
