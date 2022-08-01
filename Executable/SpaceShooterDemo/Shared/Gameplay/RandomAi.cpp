#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dWorldAccessor.hpp>

#include <Gameplay/AlignmentComponent.hpp>
#include <Gameplay/InputConstant.hpp>
#include <Gameplay/PlayerInfoSingleton.hpp>
#include <Gameplay/RandomAi.hpp>
#include <Gameplay/RandomAiComponent.hpp>
#include <Gameplay/RandomSingleton.hpp>

#include <Input/InputListenerComponent.hpp>

#include <Log/Log.hpp>

#include <Shared/Checkpoint.hpp>

namespace RandomAi
{
using namespace Emergence::Memory::Literals;

class AttachmentCreator final : public Emergence::Celerity::TaskExecutorBase<AttachmentCreator>
{
public:
    AttachmentCreator (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchPlayerInfo;
    Emergence::Celerity::FetchSequenceQuery fetchAssemblyFinishedEvents;
    Emergence::Celerity::FetchValueQuery fetchAlignmentById;
    Emergence::Celerity::InsertLongTermQuery insertRandomAi;
};

AttachmentCreator::AttachmentCreator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPlayerInfo (FETCH_SINGLETON (PlayerInfoSingleton)),
      fetchAssemblyFinishedEvents (FETCH_SEQUENCE (Emergence::Celerity::AssemblyFinishedFixedEvent)),
      fetchAlignmentById (FETCH_VALUE_1F (AlignmentComponent, objectId)),
      insertRandomAi (INSERT_LONG_TERM (RandomAiComponent))
{
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::ASSEMBLY_FINISHED);
}

void AttachmentCreator::Execute () noexcept
{
    auto playerInfoCursor = fetchPlayerInfo.Execute ();
    const auto *playerInfo = static_cast<const PlayerInfoSingleton *> (*playerInfoCursor);

    for (auto eventCursor = fetchAssemblyFinishedEvents.Execute ();
         const auto *event = static_cast<const Emergence::Celerity::AssemblyFinishedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto alignmentCursor = fetchAlignmentById.Execute (&event->objectId);
        const auto *alignment = static_cast<const AlignmentComponent *> (*alignmentCursor);

        if (alignment && alignment->playerId != playerInfo->localPlayerUid)
        {
            auto randomAiCursor = insertRandomAi.Execute ();
            auto *randomAi = static_cast<RandomAiComponent *> (++randomAiCursor);
            randomAi->objectId = event->objectId;
        }
    }
}

class InputGenerator final : public Emergence::Celerity::TaskExecutorBase<InputGenerator>
{
public:
    InputGenerator (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::ModifySingletonQuery modifyRandom;

    Emergence::Celerity::ModifyAscendingRangeQuery modifyRandomAiByIdAscending;
    Emergence::Celerity::EditValueQuery editInputListenerById;

    Emergence::Celerity::FetchValueQuery fetchTransformById;
    Emergence::Celerity::Transform3dWorldAccessor transformWorldAccessor;
};

InputGenerator::InputGenerator (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      modifyRandom (MODIFY_SINGLETON (RandomSingleton)),

      modifyRandomAiByIdAscending (MODIFY_ASCENDING_RANGE (RandomAiComponent, objectId)),
      editInputListenerById (EDIT_VALUE_1F (InputListenerComponent, objectId)),

      fetchTransformById (FETCH_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    _constructor.MakeDependencyOf (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
}

void InputGenerator::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto randomCursor = modifyRandom.Execute ();
    auto *random = static_cast<RandomSingleton *> (*randomCursor);

    for (auto randomAiCursor = modifyRandomAiByIdAscending.Execute (nullptr, nullptr);
         auto *randomAi = static_cast<RandomAiComponent *> (*randomAiCursor);)
    {
        auto transformCursor = fetchTransformById.Execute (&randomAi->objectId);
        const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            EMERGENCE_LOG (ERROR, "RandomAi: Unable to attach random ai to object with id ", randomAi->objectId,
                           " that has no Transform3dComponent!");
            ~randomAiCursor;
            continue;
        }

        auto inputListenerCursor = editInputListenerById.Execute (&randomAi->objectId);
        auto *inputListener = static_cast<InputListenerComponent *> (*inputListenerCursor);

        if (!inputListener)
        {
            EMERGENCE_LOG (ERROR, "RandomAi: Unable to attach random ai to object with id ", randomAi->objectId,
                           " that has no InputListenerComponent!");
            ~randomAiCursor;
            continue;
        }

        const Emergence::Math::Transform3d &worldTransform =
            transform->GetLogicalWorldTransform (transformWorldAccessor);
        constexpr float TARGET_TOLERANCE_SQUARED = 0.01f;

        if (time->fixedTimeNs >= randomAi->chaseTargetPointUntilNs ||
            (worldTransform.translation - randomAi->currentTargetPoint).LengthSquared () < TARGET_TOLERANCE_SQUARED)
        {
            constexpr float TARGET_STEP = 5.0f;

            randomAi->currentTargetPoint =
                worldTransform.translation +
                Emergence::Math::Vector3f {random->NextInRange (-TARGET_STEP, TARGET_STEP), 0.0f,
                                           random->NextInRange (-TARGET_STEP, TARGET_STEP)};

            constexpr uint64_t CHASE_TIME_NS = 10000000000u; // 10s
            randomAi->chaseTargetPointUntilNs = time->fixedTimeNs + CHASE_TIME_NS;
        }

        const Emergence::Math::Vector3f currentForward =
            Emergence::Math::Rotate (Emergence::Math::Vector3f::FORWARD, worldTransform.rotation);

        const Emergence::Math::Vector3f targetForward =
            (randomAi->currentTargetPoint - worldTransform.translation).Normalize ();

        const float angle = Emergence::Math::SignedAngle (currentForward, targetForward);
        InputAction rotationAction {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::ROTATION_FACTOR_ACTION};
        rotationAction.real = {0.0f, angle < 0.0f ? -1.0f : 1.0f, 0.0f};
        inputListener->actions.TryEmplaceBack (rotationAction);

        InputAction movementAction {InputConstant::MOVEMENT_ACTION_GROUP, InputConstant::MOTION_FACTOR_ACTION};
        movementAction.real = {0.0f, 0.0f, 1.0f};
        inputListener->actions.TryEmplaceBack (movementAction);

        if (random->Next () < time->fixedDurationS * randomAi->averageShotsPerS)
        {
            const InputAction action {InputConstant::FIGHT_ACTION_GROUP, InputConstant::FIRE_ACTION};
            inputListener->actions.TryEmplaceBack (action);
        }

        ++randomAiCursor;
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask ("RandomAi::AttachmentCreator"_us).SetExecutor<AttachmentCreator> ();

    _pipelineBuilder.AddTask ("RandomAi::RemoveAfterTransformRemoval"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, RandomAiComponent, objectId)
        .MakeDependencyOf ("RandomAi::InputGenerator"_us);

    _pipelineBuilder.AddTask ("RandomAi::InputGenerator"_us).SetExecutor<InputGenerator> ();
}
} // namespace RandomAi
