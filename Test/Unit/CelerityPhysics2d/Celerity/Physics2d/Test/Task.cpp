#include <Celerity/PipelineBuilderMacros.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>
#include <Celerity/Physics2d/Simulation.hpp>
#include <Celerity/Physics2d/Test/Task.hpp>
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
    std::uint64_t currentFrameIndex = 0u;
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
    : TaskExecutorBase (_constructor),

      frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      modifyPhysicsWorld (MODIFY_SINGLETON (PhysicsWorld2dSingleton)),

      insertMaterial (INSERT_LONG_TERM (DynamicsMaterial2d)),
      modifyMaterialById (MODIFY_VALUE_1F (DynamicsMaterial2d, id)),

      insertTransform (INSERT_LONG_TERM (Transform2dComponent)),
      modifyTransformById (MODIFY_VALUE_1F (Transform2dComponent, objectId)),

      insertBody (INSERT_LONG_TERM (RigidBody2dComponent)),
      modifyBodyById (MODIFY_VALUE_1F (RigidBody2dComponent, objectId)),

      insertShape (INSERT_LONG_TERM (CollisionShape2dComponent)),
      modifyShapeByShapeId (MODIFY_VALUE_1F (CollisionShape2dComponent, shapeId))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Physics2dSimulation::Checkpoint::STARTED);
}

void Configurator::Execute ()
{
    if (currentFrameIndex == 0u)
    {
        // Fill test collision mask during first frame. For simplicity, group X collides only with group X objects.
        auto worldCursor = modifyPhysicsWorld.Execute ();
        auto *physicsWorld = static_cast<PhysicsWorld2dSingleton *> (*worldCursor);

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
                    LOG ("Adding DynamicsMaterial2d with id \"", _task.id, "\".");
                    auto cursor = insertMaterial.Execute ();
                    auto *material = static_cast<DynamicsMaterial2d *> (++cursor);

#define FILL_MATERIAL                                                                                                  \
    material->id = _task.id;                                                                                           \
    material->friction = _task.friction;                                                                               \
    material->restitution = _task.restitution;                                                                         \
    material->restitutionThreshold = _task.restitutionThreshold;                                                       \
    material->density = _task.density

                    FILL_MATERIAL;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateDynamicsMaterial>)
                {
                    LOG ("Updating DynamicsMaterial2d with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    auto *material = static_cast<DynamicsMaterial2d *> (*cursor);
                    REQUIRE (material);
                    FILL_MATERIAL;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveDynamicsMaterial>)
                {
                    LOG ("Removing DynamicsMaterial2d with id \"", _task.id, "\".");
                    auto cursor = modifyMaterialById.Execute (&_task.id);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddTransform>)
                {
                    LOG ("Adding Transform2dComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertTransform.Execute ();
                    auto *transform = static_cast<Transform2dComponent *> (++cursor);
                    transform->SetObjectId (_task.objectId);
                    transform->SetLogicalLocalTransform (_task.transform);
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateTransform>)
                {
                    LOG ("Updating Transform2dComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyTransformById.Execute (&_task.objectId);
                    auto *transform = static_cast<Transform2dComponent *> (*cursor);
                    REQUIRE (transform);
                    transform->SetLogicalLocalTransform (_task.transform);
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveTransform>)
                {
                    LOG ("Removing Transform2dComponent from object with id ", _task.objectId, ".");
                    auto cursor = modifyTransformById.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddRigidBody>)
                {
                    LOG ("Adding RigidBody2dComponent to object with id ", _task.objectId, ".");
                    auto cursor = insertBody.Execute ();
                    auto *body = static_cast<RigidBody2dComponent *> (++cursor);

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
    body->fixedRotation = _task.fixedRotation

                    FILL_BODY;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateRigidBody>)
                {
                    LOG ("Update RigidBody2dComponent on object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    auto *body = static_cast<RigidBody2dComponent *> (*cursor);
                    REQUIRE (body);
                    FILL_BODY;
#undef FILL_BODY
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveRigidBody>)
                {
                    LOG ("Removing RigidBody2dComponent from object with id ", _task.objectId, ".");
                    auto cursor = modifyBodyById.Execute (&_task.objectId);
                    REQUIRE (*cursor);
                    ~cursor;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::AddCollisionShape>)
                {
                    LOG ("Adding CollisionShape2dComponent with shape id ", _task.shapeId, " to object with id ",
                         _task.objectId, ".");

                    auto cursor = insertShape.Execute ();
                    auto *shape = static_cast<CollisionShape2dComponent *> (++cursor);

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
    shape->maintainCollisionContacts = _task.maintainCollisionContacts;                                                \
    shape->collisionGroup = _task.collisionGroup

                    FILL_SHAPE;
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::UpdateCollisionShape>)
                {
                    LOG ("Update CollisionShape2dComponent with shape id ", _task.shapeId, " on object with id ",
                         _task.objectId, ".");

                    auto cursor = modifyShapeByShapeId.Execute (&_task.shapeId);
                    auto *shape = static_cast<CollisionShape2dComponent *> (*cursor);
                    REQUIRE (shape);
                    REQUIRE (shape->objectId == _task.objectId);
                    FILL_SHAPE;
#undef FILL_SHAPE
                }
                else if constexpr (std::is_same_v<Type, ConfiguratorTasks::RemoveCollisionShape>)
                {
                    LOG ("Removing CollisionShape2dComponent with shape id ", _task.shapeId, ".");
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
    std::uint64_t currentFrameIndex = 0u;
    Container::Vector<ValidatorFrame> frames;
    Container::Vector<ValidatorFrame>::const_iterator framesIterator;

    FetchValueQuery fetchBodyById;
    FetchValueQuery fetchShapeByShapeId;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchAscendingRangeQuery fetchCollisionContactByIdAscending;
    FetchAscendingRangeQuery fetchTriggerContactByIdAscending;
};

Validator::Validator (TaskConstructor &_constructor, Container::Vector<ValidatorFrame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      frames (std::move (_frames)),
      framesIterator (frames.begin ()),

      fetchBodyById (FETCH_VALUE_1F (RigidBody2dComponent, objectId)),
      fetchShapeByShapeId (FETCH_VALUE_1F (CollisionShape2dComponent, shapeId)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchCollisionContactByIdAscending (FETCH_ASCENDING_RANGE (CollisionContact2d, collisionContactId)),
      fetchTriggerContactByIdAscending (FETCH_ASCENDING_RANGE (TriggerContact2d, triggerContactId))
{
    _constructor.DependOn (Physics2dSimulation::Checkpoint::FINISHED);
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
                         _task.transform.translation.x, ", ", _task.transform.translation.y, "}, {",
                         _task.transform.rotation, "}, {", _task.transform.scale.x, ", ", _task.transform.scale.y,
                         "}.");

                    auto cursor = fetchTransformById.Execute (&_task.objectId);
                    const auto *transform = static_cast<const Transform2dComponent *> (*cursor);
                    REQUIRE (transform);

                    const Math::Transform2d &worldTransform =
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
                    CHECK (nearlyEqual (worldTransform.rotation, _task.transform.rotation, 0.001f));
                    CHECK (nearlyEqual (worldTransform.scale.x, _task.transform.scale.x, 0.00001f));
                    CHECK (nearlyEqual (worldTransform.scale.y, _task.transform.scale.y, 0.00001f));
                }
                else if constexpr (std::is_same_v<Type, ValidatorTasks::CheckContacts>)
                {
                    auto checkContacts = [] (FetchAscendingRangeQuery &_query, const auto &_expected)
                    {
                        using VectorType = std::decay_t<decltype (_expected)>;
                        using ValueType = typename VectorType::value_type;
                        VectorType found;

                        for (auto cursor = _query.Execute (nullptr, nullptr);
                             const auto *contact = static_cast<const ValueType *> (*cursor); ++cursor)
                        {
                            found.emplace_back (*contact);
                        }

                        CHECK_EQUAL (found.size (), _expected.size ());
                        for (const ValueType &contact : _expected)
                        {
                            CHECK (std::find (found.begin (), found.end (), contact) != found.end ());
                        }
                    };

                    LOG ("Checking collision contacts...");
                    checkContacts (fetchCollisionContactByIdAscending, _task.collisionContacts);

                    LOG ("Checking trigger contacts...");
                    checkContacts (fetchTriggerContactByIdAscending, _task.triggerContacts);
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
        RegisterTransform2dEvents (registrar);
        RegisterTransformCommonEvents (registrar);
    }

    PipelineBuilder builder {world.GetRootView ()};
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    Physics2dSimulation::AddToFixedUpdate (builder);
    TransformHierarchyCleanup::Add2dToFixedUpdate (builder);

    builder.AddTask ("Configurator"_us).SetExecutor<Configurator> (std::move (_configuratorFrames));
    builder.AddTask ("Validator"_us).SetExecutor<Validator> (std::move (_validatorFrames));
    REQUIRE (builder.End ());

    for (std::uint64_t frameIndex = 0u; frameIndex <= frames; ++frameIndex)
    {
        WorldTestingUtility::RunFixedUpdateOnce (world);
    }
}
} // namespace Emergence::Celerity::Test
