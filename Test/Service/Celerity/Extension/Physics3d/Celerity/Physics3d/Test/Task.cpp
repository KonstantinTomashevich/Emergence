#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>
#include <Celerity/Physics3d/Events.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/Physics3d/Simulation.hpp>
#include <Celerity/Physics3d/Test/Task.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorFrame> _frames) noexcept;

    void Execute ();

private:
    uint64_t currentFrameIndex = 0u;
    Container::Vector<ConfiguratorFrame> frames;
    Container::Vector<ConfiguratorFrame>::const_iterator framesIterator;

    ModifySingletonQuery modifyPhysicsWorld;

    InsertLongTermQuery insertMaterial;
    ModifyValueQuery modifyMaterialById;

    InsertLongTermQuery insertTransform;
    ModifyValueQuery modifyTransformById;

    InsertLongTermQuery insertBody;
    ModifyValueQuery modifyBodyById;

    InsertLongTermQuery insertShape;
    ModifyValueQuery modifyShapeByShapeId;
};

Configurator::Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorFrame> _frames) noexcept
    : frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      modifyPhysicsWorld (MODIFY_SINGLETON (PhysicsWorld3dSingleton)),

      insertMaterial (INSERT_LONG_TERM (DynamicsMaterial3d)),
      modifyMaterialById (MODIFY_VALUE_1F (DynamicsMaterial3d, id)),

      insertTransform (INSERT_LONG_TERM (Transform3dComponent)),
      modifyTransformById (MODIFY_VALUE_1F (Transform3dComponent, objectId)),

      insertBody (INSERT_LONG_TERM (RigidBody3dComponent)),
      modifyBodyById (MODIFY_VALUE_1F (RigidBody3dComponent, objectId)),

      insertShape (INSERT_LONG_TERM (CollisionShape3dComponent)),
      modifyShapeByShapeId (MODIFY_VALUE_1F (CollisionShape3dComponent, shapeId))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Physics3dSimulation::Checkpoint::STARTED);
}

void Configurator::Execute ()
{
    if (currentFrameIndex == 0u)
    {
        // Fill test collision mask during first frame. For simplicity, group X collides only with group X objects.
        auto worldCursor = modifyPhysicsWorld.Execute ();
        auto *physicsWorld = static_cast<PhysicsWorld3dSingleton *> (*worldCursor);

        // Enable memory profiling to check that it's not breaking anything.
        physicsWorld->enableMemoryProfiling = true;

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

    EMERGENCE_ASSERT (framesIterator->frameIndex == currentFrameIndex);
    for (const ConfiguratorTask &task : framesIterator->tasks)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddDynamicsMaterial>)
                {
                    LOG ("Adding DynamicsMaterial3d with id \"", _task.id, "\".");
                    auto cursor = insertMaterial.Execute ();
                    auto *material = static_cast<DynamicsMaterial3d *> (++cursor);

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
                    LOG ("Updating DynamicsMaterial3d with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    auto *material = static_cast<DynamicsMaterial3d *> (*cursor);
                    REQUIRE (material);
                    FILL_MATERIAL;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveDynamicsMaterial>)
                {
                    LOG ("Removing DynamicsMaterial3d with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddTransform>)
                {
                    LOG ("Adding Transform3dComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertTransform.Execute ();
                    auto *transform = static_cast<Transform3dComponent *> (++cursor);
                    transform->SetObjectId (_task.objectId);
                    transform->SetLogicalLocalTransform (_task.transform);
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateTransform>)
                {
                    LOG ("Updating Transform3dComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyTransformById.Execute (&_task.objectId);
                    auto *transform = static_cast<Transform3dComponent *> (*cursor);
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
                    LOG ("Adding RigidBody3dComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertBody.Execute ();
                    auto *body = static_cast<RigidBody3dComponent *> (++cursor);

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
                    LOG ("Update RigidBody3dComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    auto *body = static_cast<RigidBody3dComponent *> (*cursor);
                    REQUIRE (body);
                    FILL_BODY;
#undef FILL_BODY
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveRigidBody>)
                {
                    LOG ("Removing RigidBody3dComponent from object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddCollisionShape>)
                {
                    LOG ("Adding CollisionShape3dComponent with shape id ", _task.shapeId, " to object with id ",
                         _task.objectId, ".");

                    auto cursor = insertShape.Execute ();
                    auto *shape = static_cast<CollisionShape3dComponent *> (++cursor);

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
                    LOG ("Update CollisionShape3dComponent with shape id ", _task.shapeId, " on object with id ",
                         _task.objectId, ".");

                    auto cursor = modifyShapeByShapeId.Execute (&_task.shapeId);
                    auto *shape = static_cast<CollisionShape3dComponent *> (*cursor);
                    REQUIRE (shape);
                    REQUIRE (shape->objectId == _task.objectId);
                    FILL_SHAPE;
#undef FILL_SHAPE
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveCollisionShape>)
                {
                    LOG ("Removing CollisionShape3dComponent with shape id ", _task.shapeId, ".");
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

class Validator : public TaskExecutorBase<Validator>
{
public:
    Validator (TaskConstructor &_constructor, Container::Vector<ValidatorFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    uint64_t currentFrameIndex = 0u;
    Container::Vector<ValidatorFrame> frames;
    Container::Vector<ValidatorFrame>::const_iterator framesIterator;

    FetchValueQuery fetchBodyById;
    FetchValueQuery fetchShapeByShapeId;

    FetchValueQuery fetchTransformById;
    Transform3dWorldAccessor transformWorldAccessor;

    FetchSequenceQuery fetchContactFoundEvents;
    FetchSequenceQuery fetchContactPersistsEvents;
    FetchSequenceQuery fetchContactLostEvents;

    FetchSequenceQuery fetchTriggerEnteredEvents;
    FetchSequenceQuery fetchTriggerExitedEvents;
};

Validator::Validator (TaskConstructor &_constructor, Container::Vector<ValidatorFrame> _frames) noexcept
    : frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      fetchBodyById (FETCH_VALUE_1F (RigidBody3dComponent, objectId)),
      fetchShapeByShapeId (FETCH_VALUE_1F (CollisionShape3dComponent, shapeId)),

      fetchTransformById (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchContactFoundEvents (FETCH_SEQUENCE (Contact3dFoundEvent)),
      fetchContactPersistsEvents (FETCH_SEQUENCE (Contact3dPersistsEvent)),
      fetchContactLostEvents (FETCH_SEQUENCE (Contact3dLostEvent)),

      fetchTriggerEnteredEvents (FETCH_SEQUENCE (Trigger3dEnteredEvent)),
      fetchTriggerExitedEvents (FETCH_SEQUENCE (Trigger3dExitedEvent))
{
    _constructor.DependOn (Physics3dSimulation::Checkpoint::FINISHED);
}

void Validator::Execute () noexcept
{
    if (framesIterator == frames.end () || framesIterator->frameIndex > currentFrameIndex)
    {
        ++currentFrameIndex;
        return;
    }

    EMERGENCE_ASSERT (framesIterator->frameIndex == currentFrameIndex);
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
                    const auto *transform = static_cast<const Transform3dComponent *> (*cursor);
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
                    auto checkEvents = [] (FetchSequenceQuery &_query, const auto &_expected)
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
    World world {"TestWorld"_us, WorldConfiguration {{TEST_FIXED_FRAME_S}}};

    {
        EventRegistrar registrar {&world};
        RegisterPhysicsEvents (registrar);
        RegisterTransform3dEvents (registrar);
        RegisterTransformCommonEvents (registrar);
    }

    PipelineBuilder builder {world.GetRootView ()};
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    Physics3dSimulation::AddToFixedUpdate (builder);
    TransformHierarchyCleanup::Add3dToFixedUpdate (builder);

    builder.AddTask ("Configurator"_us).SetExecutor<Configurator> (std::move (_configuratorFrames));
    builder.AddTask ("Validator"_us).SetExecutor<Validator> (std::move (_validatorFrames));
    REQUIRE (builder.End ());

    for (std::uint64_t frameIndex = 0u; frameIndex <= frames; ++frameIndex)
    {
        WorldTestingUtility::RunFixedUpdateOnce (world);
    }
}
} // namespace Emergence::Celerity::Test
