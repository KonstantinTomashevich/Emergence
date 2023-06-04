#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Log/Log.hpp>

#include <Platformer/Animation/CharacterAnimation.hpp>
#include <Platformer/Animation/CharacterAnimationConfiguration.hpp>
#include <Platformer/Animation/CharacterAnimationControllerComponent.hpp>
#include <Platformer/Movement/MovementComponent.hpp>

namespace CharacterAnimation
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"CharacterAnimation::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"CharacterAnimation::Finished"};

class Controller final : public Emergence::Celerity::TaskExecutorBase<Controller>
{
public:
    Controller (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void EnsureSpriteAnimationExists (Emergence::Celerity::UniqueId _objectId,
                                      Emergence::Celerity::UniqueId _spriteId) noexcept;

    CharacterAnimationState ExtractStateAndDirectionFromMovement (Emergence::Celerity::UniqueId _objectId,
                                                                  MovementDirection &_newDirection) noexcept;

    Emergence::Celerity::ModifyAscendingRangeQuery modifyCharacterAnimationControllerByIdAscending;

    Emergence::Celerity::InsertLongTermQuery insertSpriteAnimation;
    Emergence::Celerity::FetchValueQuery fetchSpriteAnimationBySpriteId;
    Emergence::Celerity::EditValueQuery editSpriteAnimationBySpriteId;

    Emergence::Celerity::FetchValueQuery fetchMovementById;
    Emergence::Celerity::FetchValueQuery fetchCharacterAnimationConfigurationById;
};

Controller::Controller (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyCharacterAnimationControllerByIdAscending (
          MODIFY_ASCENDING_RANGE (CharacterAnimationControllerComponent, objectId)),

      insertSpriteAnimation (INSERT_LONG_TERM (Emergence::Celerity::Sprite2dUvAnimationComponent)),
      fetchSpriteAnimationBySpriteId (FETCH_VALUE_1F (Emergence::Celerity::Sprite2dUvAnimationComponent, spriteId)),
      editSpriteAnimationBySpriteId (EDIT_VALUE_1F (Emergence::Celerity::Sprite2dUvAnimationComponent, spriteId)),

      fetchMovementById (FETCH_VALUE_1F (MovementComponent, objectId)),
      fetchCharacterAnimationConfigurationById (FETCH_VALUE_1F (CharacterAnimationConfiguration, id))
{
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
}

void Controller::Execute () noexcept
{
    for (auto controllerCursor = modifyCharacterAnimationControllerByIdAscending.Execute (nullptr, nullptr);
         auto *controller = static_cast<CharacterAnimationControllerComponent *> (*controllerCursor);)
    {
        MovementDirection newDirection = controller->direction;
        const CharacterAnimationState newState =
            ExtractStateAndDirectionFromMovement (controller->objectId, newDirection);

        if (newState != controller->currentState || newDirection != controller->direction)
        {
            controller->direction = newDirection;
            controller->currentState = newState;

            auto configurationCursor =
                fetchCharacterAnimationConfigurationById.Execute (&controller->animationConfigurationId);
            const auto *configuration = static_cast<const CharacterAnimationConfiguration *> (*configurationCursor);

            if (!configuration)
            {
                EMERGENCE_LOG (
                    ERROR, "CharacterAnimation: Deleting CharacterAnimationControllerComponent from object with id ",
                    controller->objectId, " as it has invalid configuration \"", controller->animationConfigurationId,
                    "\"!");
                ~controllerCursor;
                continue;
            }

            EnsureSpriteAnimationExists (controller->objectId, controller->spriteId);
            auto animationCursor = editSpriteAnimationBySpriteId.Execute (&controller->spriteId);
            auto *animation = static_cast<Emergence::Celerity::Sprite2dUvAnimationComponent *> (*animationCursor);

            switch (newState)
            {
            case CharacterAnimationState::NONE:
                animation->animationId = {};
                break;

            case CharacterAnimationState::CROUCH:
                animation->animationId = configuration->crouchAnimationId;
                break;

            case CharacterAnimationState::FALL:
                animation->animationId = configuration->fallAnimationId;
                break;

            case CharacterAnimationState::IDLE:
                animation->animationId = configuration->idleAnimationId;
                break;

            case CharacterAnimationState::JUMP:
                animation->animationId = configuration->jumpAnimationId;
                break;

            case CharacterAnimationState::ROLL:
                animation->animationId = configuration->rollAnimationId;
                break;

            case CharacterAnimationState::RUN:
                animation->animationId = configuration->runAnimationId;
                break;

            case CharacterAnimationState::SLIDE:
                animation->animationId = configuration->slideAnimationId;
                break;
            }

            animation->flipU = controller->direction == MovementDirection::LEFT;
            animation->currentTimeNs = 0u;
        }

        ++controllerCursor;
    }
}

void Controller::EnsureSpriteAnimationExists (Emergence::Celerity::UniqueId _objectId,
                                              Emergence::Celerity::UniqueId _spriteId) noexcept
{
    if (auto cursor = fetchSpriteAnimationBySpriteId.Execute (&_spriteId); *cursor)
    {
        return;
    }

    auto cursor = insertSpriteAnimation.Execute ();
    auto *animation = static_cast<Emergence::Celerity::Sprite2dUvAnimationComponent *> (++cursor);
    animation->objectId = _objectId;
    animation->spriteId = _spriteId;
    animation->tickTime = true;
    animation->loop = true;
}

CharacterAnimationState Controller::ExtractStateAndDirectionFromMovement (Emergence::Celerity::UniqueId _objectId,
                                                                          MovementDirection &_newDirection) noexcept
{
    if (auto movementCursor = fetchMovementById.Execute (&_objectId);
        const auto *movement = static_cast<const MovementComponent *> (*movementCursor))
    {
        _newDirection = movement->lastMovementDirection;

        // Movement dictates animation state.
        switch (movement->state)
        {
        case MovementState::IDLE:
        case MovementState::BLOCKED:
            return CharacterAnimationState::IDLE;

        case MovementState::RUN:
            return CharacterAnimationState::RUN;

        case MovementState::CROUCH:
            return CharacterAnimationState::CROUCH;

        case MovementState::JUMP:
            return CharacterAnimationState::JUMP;

        case MovementState::FALL:
            return CharacterAnimationState::FALL;

        case MovementState::ROLL:
            return CharacterAnimationState::ROLL;

        case MovementState::SLIDE:
            return CharacterAnimationState::SLIDE;
        }
    }

    EMERGENCE_ASSERT (false);
    return CharacterAnimationState::IDLE;
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("CharacterAnimation::RemoveControllerComponent"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupNormalEvent,
                                CharacterAnimationControllerComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("CharacterAnimationController"_us).SetExecutor<Controller> ();
}
} // namespace CharacterAnimation
