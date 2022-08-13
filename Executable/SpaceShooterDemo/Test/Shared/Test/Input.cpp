#include <thread>
#include <variant>

#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Transform3dHierarchyCleanup.hpp>

#include <Container/Vector.hpp>

#include <Log/Log.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerComponent.hpp>
#include <Input/InputSingleton.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Test/Input.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity;

using namespace Emergence::Container;
using namespace Emergence::Memory::Literals;

bool InputTestIncludeMarker () noexcept
{
    return true;
}

namespace Steps
{
struct CreateListener
{
    UniqueId id = INVALID_UNIQUE_ID;
};

struct DeleteListener
{
    UniqueId id = INVALID_UNIQUE_ID;
};

struct AddSubscription
{
    InputSubscription subscription;
    bool normal = true;
};

struct UnsubscribeListener
{
    UniqueId id = INVALID_UNIQUE_ID;
    bool normal = true;
};

struct UnsubscribeGroup
{
    Emergence::Memory::UniqueString id;
};

struct FireKeyDown
{
    ScanCode scan;
    QualifiersMask qualifiers;
};

struct FireKeyUp
{
    ScanCode scan;
    QualifiersMask qualifiers;
};

struct PushCustomAction
{
    InputAction action;
    bool instant;
};
} // namespace Steps

using ConfiguratorStep = std::variant<Steps::CreateListener,
                                      Steps::DeleteListener,
                                      Steps::AddSubscription,
                                      Steps::UnsubscribeListener,
                                      Steps::UnsubscribeGroup,
                                      Steps::FireKeyDown,
                                      Steps::FireKeyUp,
                                      Steps::PushCustomAction>;

using FrameConfiguration = Vector<ConfiguratorStep>;

class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor,
                  Vector<FrameConfiguration> _steps,
                  Vector<KeyStateTrigger> _keyStateTriggers,
                  Vector<KeyStateChangedTrigger> _keyStateChangedTriggers,
                  InputAccumulator *_eventOutput);

    void Execute ();

private:
    Vector<FrameConfiguration> steps;
    Vector<KeyStateTrigger> keyStateTriggers;
    Vector<KeyStateChangedTrigger> keyStateChangedTriggers;
    std::size_t framesConfigured = 0u;

    InputAccumulator *eventOutput;
    Emergence::Celerity::InsertLongTermQuery createListener;
    Emergence::Celerity::RemoveValueQuery removeListenerById;
    Emergence::Celerity::ModifySingletonQuery modifyInput;
};

Configurator::Configurator (TaskConstructor &_constructor,
                            Vector<FrameConfiguration> _steps,
                            Vector<KeyStateTrigger> _keyStateTriggers,
                            Vector<KeyStateChangedTrigger> _keyStateChangedTriggers,
                            InputAccumulator *_eventOutput)
    : steps (std::move (_steps)),
      keyStateTriggers (std::move (_keyStateTriggers)),
      keyStateChangedTriggers (std::move (_keyStateChangedTriggers)),
      eventOutput (_eventOutput),
      createListener (INSERT_LONG_TERM (InputListenerComponent)),
      removeListenerById (REMOVE_VALUE_1F (InputListenerComponent, objectId)),
      modifyInput (MODIFY_SINGLETON (InputSingleton))
{
    REQUIRE (eventOutput);
    _constructor.MakeDependencyOf (Input::Checkpoint::DISPATCH_STARTED);
}

void Configurator::Execute ()
{
    REQUIRE (framesConfigured < steps.size ());
    EMERGENCE_LOG (INFO, "[Configurator] Configuration sequence ", framesConfigured);

    auto cursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*cursor);

    // Initialize triggers if they are not initialized already.

    if (input->keyStateTriggers.Empty ())
    {
        for (const KeyStateTrigger &trigger : keyStateTriggers)
        {
            REQUIRE (input->keyStateTriggers.TryEmplaceBack (trigger));
        }
    }

    if (input->keyStateChangedTriggers.Empty ())
    {
        for (const KeyStateChangedTrigger &trigger : keyStateChangedTriggers)
        {
            REQUIRE (input->keyStateChangedTriggers.TryEmplaceBack (trigger));
        }
    }

    // Process steps for current frame.

    for (const ConfiguratorStep &step : steps[framesConfigured])
    {
        std::visit (
            [this, input] (const auto &_step)
            {
                using Type = std::decay_t<decltype (_step)>;
                if constexpr (std::is_same_v<Type, Steps::CreateListener>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Create listener ", _step.id, ".");
                    auto cursor = createListener.Execute ();
                    (new (++cursor) InputListenerComponent ())->objectId = _step.id;
                }
                else if constexpr (std::is_same_v<Type, Steps::DeleteListener>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Delete listener ", _step.id, ".");
                    auto cursor = removeListenerById.Execute (&_step.id);
                    REQUIRE (cursor.ReadConst ());
                    ~cursor;
                    REQUIRE (!cursor.ReadConst ());
                }
                else if constexpr (std::is_same_v<Type, Steps::AddSubscription>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Subscribe listener ", _step.subscription.listenerId,
                                   " to group \"", _step.subscription.group, "\" in ",
                                   _step.normal ? "normal" : "fixed", " update.");

                    InputSingleton::SubscriptionVector &subscriptions =
                        _step.normal ? input->normalSubscriptions : input->fixedSubscriptions;

                    REQUIRE (subscriptions.TryEmplaceBack (_step.subscription));
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeListener>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Unsubscribe listener ", _step.id, "\" in ",
                                   _step.normal ? "normal" : "fixed", " update.");

                    if (_step.normal)
                    {
                        input->UnsubscribeNormal (_step.id);
                    }
                    else
                    {
                        input->UnsubscribeFixed (_step.id);
                    }
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeGroup>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Unsubscribe all listeners from group \"", _step.id, "\".");
                    input->UnsubscribeGroup (_step.id);
                }
                else if constexpr (std::is_same_v<Type, Steps::FireKeyDown>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Fire key down event for \"", _step.scan,
                                   "\" with qualifiers mask ", _step.qualifiers, ".");

                    InputEvent event;
                    event.type = InputType::KEYBOARD;
                    // For simplicity, we assume that KeyCode == ScanCode here,
                    // as it is on QWERTY keyboards on English layout.
                    event.keyboard = {_step.scan, _step.scan, true, _step.qualifiers};
                    eventOutput->PostEvent (event);
                }
                else if constexpr (std::is_same_v<Type, Steps::FireKeyUp>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Fire key up event for \"", _step.scan,
                                   "\" with qualifiers mask ", _step.qualifiers, ".");

                    InputEvent event;
                    event.type = InputType::KEYBOARD;
                    // For simplicity, we assume that KeyCode == ScanCode here,
                    // as it is on QWERTY keyboards on English layout.
                    event.keyboard = {_step.scan, _step.scan, false, _step.qualifiers};
                    eventOutput->PostEvent (event);
                }
                else if constexpr (std::is_same_v<Type, Steps::PushCustomAction>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Push custom action \"", _step.action.id, "\" with ",
                                   _step.instant ? "instant" : "persistent", " mode.");

                    input->normalActionsBuffer.TryEmplaceBack (_step.action);
                    if (_step.instant)
                    {
                        input->fixedInstantActionsBuffer.TryEmplaceBack (_step.action);
                    }
                    else
                    {
                        input->fixedPersistentActionsBuffer.TryEmplaceBack (_step.action);
                    }
                }
            },
            step);
    }

    ++framesConfigured;
}

void AddConfiguratorTask (PipelineBuilder &_pipelineBuilder,
                          Vector<FrameConfiguration> _steps,
                          Vector<KeyStateChangedTrigger> _keyStateTriggers,
                          Vector<KeyStateTrigger> _keyStateChangedTriggers,
                          InputAccumulator *_eventOutput)
{
    TaskConstructor constructor = _pipelineBuilder.AddTask ("Configurator"_us);
    constructor.SetExecutor<Configurator> (std::move (_steps), std::move (_keyStateChangedTriggers),
                                           std::move (_keyStateTriggers), _eventOutput);
}

using FrameExpectation = Vector<std::pair<std::int64_t, Vector<InputAction>>>;

class Validator final : public TaskExecutorBase<Validator>
{
public:
    Validator (TaskConstructor &_constructor, Vector<FrameExpectation> _expectations) noexcept;

    void Execute () noexcept;

private:
    std::size_t framesValidated = 0u;
    Vector<FrameExpectation> expectations;

    Emergence::Celerity::FetchValueQuery fetchListenerById;
};

Validator::Validator (TaskConstructor &_constructor, Vector<FrameExpectation> _expectations) noexcept
    : expectations (std::move (_expectations)),
      fetchListenerById (FETCH_VALUE_1F (InputListenerComponent, objectId))
{
    _constructor.DependOn (Input::Checkpoint::LISTENERS_READ_ALLOWED);
}

void Validator::Execute () noexcept
{
    REQUIRE (framesValidated < expectations.size ());
    for (const auto &[objectId, actions] : expectations[framesValidated])
    {
        auto cursor = fetchListenerById.Execute (&objectId);
        const auto *listener = static_cast<const InputListenerComponent *> (*cursor);

        CHECK_EQUAL (listener->actions.GetCount (), actions.size ());
        const std::size_t actionsToCheck = std::min (listener->actions.GetCount (), actions.size ());

        // For code simplicity we check action order strictly, but in general any order of expected actions is correct.
        for (std::size_t index = 0u; index < actionsToCheck; ++index)
        {
            CHECK_EQUAL (listener->actions[index], actions[index]);
        }

        ++cursor;
        CHECK_EQUAL (*cursor, nullptr);
    }

    ++framesValidated;
}

void AddValidatorTask (PipelineBuilder &_pipelineBuilder, Vector<FrameExpectation> _expectations)
{
    TaskConstructor constructor = _pipelineBuilder.AddTask ("Validator"_us);
    constructor.SetExecutor<Validator> (std::move (_expectations));
}

struct NormalUpdateRequest final
{
    FrameConfiguration configuration;
    FrameExpectation expectation;
};

struct FixedUpdateRequest final
{
    FrameExpectation expectation;
};

void RunTest (Vector<KeyStateTrigger> _keyStateChangedTriggers,
              Vector<KeyStateChangedTrigger> _keyStateTriggers,
              const Vector<std::variant<NormalUpdateRequest, FixedUpdateRequest>> &_updates)
{
    World world {"TestWorld"_us};
    InputAccumulator inputAccumulator;
    PipelineBuilder pipelineBuilder {&world};

    // In test definitions it's much more convenient to read full frame update configs than three separate vectors,
    // therefore we accept composite vector as parameter and split it inside this helper.

    Vector<FrameConfiguration> normalConfiguration;
    Vector<FrameExpectation> normalExpectations;
    Vector<FrameExpectation> fixedExpectations;

    for (const auto &request : _updates)
    {
        std::visit (
            [&normalConfiguration, &normalExpectations, &fixedExpectations] (const auto &_request)
            {
                using Type = std::decay_t<decltype (_request)>;
                if constexpr (std::is_same_v<Type, NormalUpdateRequest>)
                {
                    normalConfiguration.emplace_back (_request.configuration);
                    normalExpectations.emplace_back (_request.expectation);
                }
                else if constexpr (std::is_same_v<Type, FixedUpdateRequest>)
                {
                    fixedExpectations.emplace_back (_request.expectation);
                }
            },
            request);
    }

    pipelineBuilder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    AddConfiguratorTask (pipelineBuilder, std::move (normalConfiguration), std::move (_keyStateTriggers),
                         std::move (_keyStateChangedTriggers), &inputAccumulator);
    AddValidatorTask (pipelineBuilder, std::move (normalExpectations));

    Input::AddToNormalUpdate (&inputAccumulator, pipelineBuilder);
    pipelineBuilder.AddCheckpoint (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_STARTED);
    pipelineBuilder.End ();

    pipelineBuilder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    AddValidatorTask (pipelineBuilder, std::move (fixedExpectations));
    Input::AddToFixedUpdate (pipelineBuilder);
    pipelineBuilder.End ();

    for (const auto &request : _updates)
    {
        std::visit (
            [&world] (const auto &_request)
            {
                using Type = std::decay_t<decltype (_request)>;
                if constexpr (std::is_same_v<Type, NormalUpdateRequest>)
                {
                    WorldTestingUtility::RunNormalUpdateOnce (world, 16666667u);
                }
                else if constexpr (std::is_same_v<Type, FixedUpdateRequest>)
                {
                    WorldTestingUtility::RunFixedUpdateOnce (world);
                }
            },
            request);

        inputAccumulator.Clear ();
    }
}

BEGIN_SUITE (Input)

TEST_CASE (SubscriptionManagement)
{
    InputAction aDown {"A"_us, "ADown"_us};
    InputAction bDown {"B"_us, "BDown"_us};

    RunTest (
        {
            KeyStateTrigger {aDown, 'a', true},
            KeyStateTrigger {bDown, 'b', true},
        },
        {},
        {
            NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"A"_us, 0u}},
                                  Steps::CreateListener {1u}, Steps::AddSubscription {{"B"_us, 1u}}},
                                 {}},
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyDown {'b', 0u}},
                                 {{0u, {aDown}}, {1u, {bDown}}}},
            NormalUpdateRequest {{Steps::AddSubscription {{"B"_us, 0u}}}, {{0u, {aDown, bDown}}, {1u, {bDown}}}},
            NormalUpdateRequest {{Steps::UnsubscribeGroup {"B"_us}}, {{0u, {aDown}}, {1u, {}}}},
            NormalUpdateRequest {{Steps::AddSubscription {{"B"_us, 0u}}, Steps::AddSubscription {{"B"_us, 1u}}},
                                 {{0u, {aDown, bDown}}, {1u, {bDown}}}},
            NormalUpdateRequest {{Steps::UnsubscribeListener {0u}}, {{0u, {}}, {1u, {bDown}}}},
        });
}

TEST_CASE (KeyStateTrigger)
{
    InputAction actionDown {"Test"_us, "Down"_us};
    InputAction actionUp {"Test"_us, "Up"_us};

    RunTest (
        {
            KeyStateTrigger {actionDown, 'a', true},
            KeyStateTrigger {actionUp, 'a', false},
        },
        {},
        {
            NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}}}, {}},

            // Key is up.
            NormalUpdateRequest {{}, {{0u, {actionUp}}}},

            // Key was both up and down: fire both actions.
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyUp {'a', 0u}},
                                 {{0u, {actionDown, actionUp}}}},

            // Key was both up and down several times: fire both actions, but do not duplicate.
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyUp {'a', 0u},
                                  Steps::FireKeyDown {'a', 0u}, Steps::FireKeyUp {'a', 0u}},
                                 {{0u, {actionDown, actionUp}}}},

            // Press and hold.
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}}, {{0u, {actionDown}}}},
            NormalUpdateRequest {{}, {{0u, {actionDown}}}},
            NormalUpdateRequest {{}, {{0u, {actionDown}}}},
            NormalUpdateRequest {{Steps::FireKeyUp {'a', 0u}}, {{0u, {actionUp}}}},
        });
}

TEST_CASE (OnePersistentActionFromSeveralTriggers)
{
    InputAction action {"Test"_us, "Down"_us};
    RunTest (
        {
            KeyStateTrigger {action, 'a', true},
            KeyStateTrigger {action, 'b', true},
        },
        {},
        {
            NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}}}, {}},
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyDown {'b', 0u}}, {{0u, {action}}}},
        });
}

TEST_CASE (KeyStateChangedTrigger)
{
    InputAction actionPressed {"Test"_us, "Pressed"_us};
    InputAction actionReleased {"Test"_us, "Released"_us};

    constexpr QualifiersMask ACTION_MASK = 8u;
    InputAction actionPressedWithQualifiers {"Test"_us, "Pressed"_us};

    RunTest ({},
             {
                 KeyStateChangedTrigger {actionPressed, 'a', true, 0u},
                 KeyStateChangedTrigger {actionReleased, 'a', false, 0u},
                 KeyStateChangedTrigger {actionPressedWithQualifiers, 'a', true, ACTION_MASK},
             },
             {
                 NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}}}, {}},

                 // Just press and release immediately.
                 NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyUp {'a', 0u}},
                                      {{0u, {actionPressed, actionReleased}}}},

                 // Press and hold.
                 NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}}, {{0u, {actionPressed}}}},
                 NormalUpdateRequest {{}, {}},
                 NormalUpdateRequest {{Steps::FireKeyUp {'a', 0u}}, {{0u, {actionReleased}}}},

                 // Check trigger with qualifier.
                 NormalUpdateRequest {{Steps::FireKeyDown {'a', ACTION_MASK}}, {{0u, {actionPressedWithQualifiers}}}},
             });
}

TEST_CASE (InstantActionDuplication)
{
    InputAction action {"Test"_us, "Pressed"_us};
    RunTest ({},
             {
                 KeyStateChangedTrigger {action, 'a', true, 0u},
                 KeyStateChangedTrigger {action, 'b', true, 0u},
             },
             {
                 NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}}}, {}},
                 NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}, Steps::FireKeyDown {'b', 0u}},
                                      {{0u, {action, action}}}},
             });
}

TEST_CASE (FixedUpdateDispatch)
{
    InputAction persistentAction {"Test"_us, "Down"_us};
    InputAction instantAction {"Test"_us, "Pressed"_us};

    RunTest (
        {KeyStateTrigger {persistentAction, 'a', true}}, {KeyStateChangedTrigger {instantAction, 'a', true, 0u}},
        {
            NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}, false}}, {}},
            NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u}}, {}},
            FixedUpdateRequest {{{0u, {persistentAction, instantAction}}}},
            FixedUpdateRequest {{{0u, {persistentAction}}}},
            NormalUpdateRequest {{}, {}},
            FixedUpdateRequest {{{0u, {persistentAction}}}},
            FixedUpdateRequest {{{0u, {persistentAction}}}},
            NormalUpdateRequest {{Steps::FireKeyUp {'a', 0u}}, {}},
            FixedUpdateRequest {{{0u, {}}}},
        });
}

TEST_CASE (PushCustomAction)
{
    InputAction persistentAction {"Test"_us, "Down"_us};
    InputAction instantAction {"Test"_us, "Pressed"_us};

    RunTest ({}, {},
             {
                 NormalUpdateRequest {
                     {Steps::CreateListener {0u}, Steps::CreateListener {1u},
                      Steps::AddSubscription {{"Test"_us, 0u}, true}, Steps::AddSubscription {{"Test"_us, 1u}, false}},
                     {}},
                 NormalUpdateRequest {
                     {Steps::PushCustomAction {persistentAction, false}, Steps::PushCustomAction {instantAction, true}},
                     {{0u, {persistentAction, instantAction}}}},
                 FixedUpdateRequest {{{1u, {persistentAction, instantAction}}}},

                 // Do second cycle to check how buffer cleanup works with custom actions.

                 NormalUpdateRequest {
                     {Steps::PushCustomAction {persistentAction, false}, Steps::PushCustomAction {instantAction, true}},
                     {{0u, {persistentAction, instantAction}}}},
                 FixedUpdateRequest {{{1u, {persistentAction, instantAction}}}},
             });
}

END_SUITE
