#include <SyntaxSugar/MuteWarnings.hpp>

#include <thread>
#include <variant>

#include <Container/Vector.hpp>

#include <Log/Log.hpp>

#include <Input/Input.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/InputSingleton.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

BEGIN_MUTING_WARNINGS
#include <OgreInput.h>
END_MUTING_WARNINGS

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Test/Input.hpp>
#include <Test/SharedApplicationContext.hpp>

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
    std::uintptr_t id = INVALID_OBJECT_ID;
};

struct DeleteListener
{
    std::uintptr_t id = INVALID_OBJECT_ID;
};

struct AddSubscription
{
    InputSubscription subscription;
    bool normal = true;
};

struct UnsubscribeListener
{
    std::uintptr_t id = INVALID_OBJECT_ID;
    bool normal = true;
};

struct UnsubscribeGroup
{
    Emergence::Memory::UniqueString id;
};

struct FireKeyDown
{
    KeyCode key;
    QualifiersMask qualifiers;
    bool repeat = false;
};

struct FireKeyUp
{
    KeyCode key;
    QualifiersMask qualifiers;
};
} // namespace Steps

using ConfiguratorStep = std::variant<Steps::CreateListener,
                                      Steps::DeleteListener,
                                      Steps::AddSubscription,
                                      Steps::UnsubscribeListener,
                                      Steps::UnsubscribeGroup,
                                      Steps::FireKeyDown,
                                      Steps::FireKeyUp>;

using FrameConfiguration = Emergence::Container::Vector<ConfiguratorStep>;

class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor,
                  Emergence::Container::Vector<FrameConfiguration> _steps,
                  Emergence::Container::Vector<KeyStateTrigger> _keyStateTriggers,
                  Emergence::Container::Vector<KeyStateChangedTrigger> _keyStateChangedTriggers);

    void Execute ();

private:
    Emergence::Container::Vector<FrameConfiguration> steps;
    Emergence::Container::Vector<KeyStateTrigger> keyStateTriggers;
    Emergence::Container::Vector<KeyStateChangedTrigger> keyStateChangedTriggers;
    std::size_t framesConfigured = 0u;

    Emergence::Warehouse::InsertLongTermQuery createListener;
    Emergence::Warehouse::ModifyValueQuery modifyListenerById;
    Emergence::Warehouse::ModifySingletonQuery modifyInput;
};

Configurator::Configurator (TaskConstructor &_constructor,
                            Emergence::Container::Vector<FrameConfiguration> _steps,
                            Emergence::Container::Vector<KeyStateTrigger> _keyStateTriggers,
                            Emergence::Container::Vector<KeyStateChangedTrigger> _keyStateChangedTriggers)
    : steps (std::move (_steps)),
      keyStateTriggers (std::move (_keyStateTriggers)),
      keyStateChangedTriggers (std::move (_keyStateChangedTriggers)),
      createListener (_constructor.InsertLongTerm (InputListenerObject::Reflect ().mapping)),
      modifyListenerById (_constructor.ModifyValue (InputListenerObject::Reflect ().mapping,
                                                    {InputListenerObject::Reflect ().objectId})),
      modifyInput (_constructor.ModifySingleton (InputSingleton::Reflect ().mapping))
{
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_STARTED);
}

void Configurator::Execute ()
{
    REQUIRE (framesConfigured < steps.size ());
    EMERGENCE_LOG (INFO, "[Configurator] Sequence ", framesConfigured);

    auto cursor = modifyInput.Execute ();
    auto *input = static_cast<InputSingleton *> (*cursor);

    // Initialize triggers if there are not initialized already.

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
                    (new (++cursor) InputListenerObject ())->objectId = _step.id;
                }
                else if constexpr (std::is_same_v<Type, Steps::DeleteListener>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Delete listener ", _step.id, ".");
                    auto cursor = modifyListenerById.Execute (&_step.id);
                    REQUIRE (*cursor);
                    ~cursor;
                    REQUIRE (!*cursor);
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
                    EMERGENCE_LOG (INFO, "[Configurator] Fire key down event for \"", _step.key,
                                   "\" with qualifiers mask ", _step.qualifiers,
                                   _step.repeat ? " with repeat flag." : " without repeat flag.");

                    OgreBites::Event event;
                    event.key.type = static_cast<int> (OgreBites::KEYDOWN);
                    event.key.keysym = {static_cast<OgreBites::Keycode> (_step.key),
                                        static_cast<unsigned short> (_step.qualifiers)};

                    event.key.repeat = _step.repeat ? 1u : 0u;
                    SharedApplicationContext::Get ()->_fireInputEvent (event, 0u);
                }
                else if constexpr (std::is_same_v<Type, Steps::FireKeyUp>)
                {
                    EMERGENCE_LOG (INFO, "[Configurator] Fire key up event for \"", _step.key,
                                   "\" with qualifiers mask ", _step.qualifiers, ".");

                    OgreBites::Event event;
                    event.key.type = static_cast<int> (OgreBites::KEYUP);
                    event.key.keysym = {static_cast<OgreBites::Keycode> (_step.key),
                                        static_cast<unsigned short> (_step.qualifiers)};

                    event.key.repeat = 0u;
                    SharedApplicationContext::Get ()->_fireInputEvent (event, 0u);
                }
            },
            step);
    }

    ++framesConfigured;
}

void AddConfiguratorTask (PipelineBuilder &_pipelineBuilder,
                          Emergence::Container::Vector<FrameConfiguration> _steps,
                          Emergence::Container::Vector<KeyStateChangedTrigger> _keyStateTriggers,
                          Emergence::Container::Vector<KeyStateTrigger> _keyStateChangedTriggers)
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("Configurator"_us);
    constructor.SetExecutor<Configurator> (std::move (_steps), std::move (_keyStateChangedTriggers),
                                           std::move (_keyStateTriggers));
}

using FrameExpectation =
    Emergence::Container::Vector<std::pair<std::int64_t, Emergence::Container::Vector<InputAction>>>;

class Validator final : public TaskExecutorBase<Validator>
{
public:
    Validator (TaskConstructor &_constructor, Emergence::Container::Vector<FrameExpectation> _expectations) noexcept;

    void Execute () noexcept;

private:
    std::size_t framesValidated = 0u;
    Emergence::Container::Vector<FrameExpectation> expectations;

    Emergence::Warehouse::FetchValueQuery fetchListenerById;
};

Validator::Validator (TaskConstructor &_constructor,
                      Emergence::Container::Vector<FrameExpectation> _expectations) noexcept
    : expectations (std::move (_expectations)),
      fetchListenerById (
          _constructor.FetchValue (InputListenerObject::Reflect ().mapping, {InputListenerObject::Reflect ().objectId}))
{
    _constructor.DependOn (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
}

void Validator::Execute () noexcept
{
    REQUIRE (framesValidated < expectations.size ());
    for (const auto &[objectId, actions] : expectations[framesValidated])
    {
        auto cursor = fetchListenerById.Execute (&objectId);
        const auto *listener = static_cast<const InputListenerObject *> (*cursor);

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

void AddValidatorTask (PipelineBuilder &_pipelineBuilder, Emergence::Container::Vector<FrameExpectation> _expectations)
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("Validator"_us);
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

void RunTest (Emergence::Container::Vector<KeyStateTrigger> _keyStateChangedTriggers,
              Emergence::Container::Vector<KeyStateChangedTrigger> _keyStateTriggers,
              const Emergence::Container::Vector<std::variant<NormalUpdateRequest, FixedUpdateRequest>> &_updates)
{
    World world {"TestWorld"_us};
    PipelineBuilder pipelineBuilder {&world};

    // In test definitions it's much more convenient to read full frame update configs than three separate vectors,
    // therefore we accept composite vector as parameter and split it inside this helper.

    Emergence::Container::Vector<FrameConfiguration> normalConfiguration;
    Emergence::Container::Vector<FrameExpectation> normalExpectations;
    Emergence::Container::Vector<FrameExpectation> fixedExpectations;

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

    pipelineBuilder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    AddConfiguratorTask (pipelineBuilder, std::move (normalConfiguration), std::move (_keyStateTriggers),
                         std::move (_keyStateChangedTriggers));
    AddValidatorTask (pipelineBuilder, std::move (normalExpectations));

    Input::AddToNormalUpdate (SharedApplicationContext::Get (), pipelineBuilder);
    AddAllCheckpoints (pipelineBuilder);
    Pipeline *normalUpdate = pipelineBuilder.End (std::thread::hardware_concurrency ());

    pipelineBuilder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    AddValidatorTask (pipelineBuilder, std::move (fixedExpectations));
    Input::AddToFixedUpdate (pipelineBuilder);
    AddAllCheckpoints (pipelineBuilder);
    Pipeline *fixedUpdate = pipelineBuilder.End (std::thread::hardware_concurrency ());

    for (const auto &request : _updates)
    {
        std::visit (
            [normalUpdate, fixedUpdate] (const auto &_request)
            {
                using Type = std::decay_t<decltype (_request)>;
                if constexpr (std::is_same_v<Type, NormalUpdateRequest>)
                {
                    normalUpdate->Execute ();
                }
                else if constexpr (std::is_same_v<Type, FixedUpdateRequest>)
                {
                    fixedUpdate->Execute ();
                }
            },
            request);
    }
}

BEGIN_SUITE (Input)

TEST_CASE (SubscriptionManagement)
{
    InputAction aDown {"A"_us, "ADown"_us};
    InputAction bDown {"B"_us, "BDown"_us};

    RunTest ({
                 KeyStateTrigger {aDown, 'a', true},
                 KeyStateTrigger {bDown, 'b', true},
             },
             {},
             {
                 NormalUpdateRequest {{Steps::CreateListener {0u}, Steps::AddSubscription {{"A"_us, 0u}},
                                       Steps::CreateListener {1u}, Steps::AddSubscription {{"B"_us, 1u}}},
                                      {}},
                 NormalUpdateRequest {{Steps::FireKeyDown {'a', 0u, false}, Steps::FireKeyDown {'b', 0u, false}},
                                      {{0u, {aDown}}, {1u, {bDown}}}},
                 NormalUpdateRequest {{Steps::AddSubscription {{"B"_us, 0u}}}, {{0u, {aDown, bDown}}, {1u, {bDown}}}},
                 NormalUpdateRequest {{Steps::UnsubscribeGroup {"B"_us}}, {{0u, {aDown}}, {1u, {}}}},
                 NormalUpdateRequest {{Steps::AddSubscription {{"B"_us, 0u}}, Steps::AddSubscription {{"B"_us, 1u}}},
                                      {{0u, {aDown, bDown}}, {1u, {bDown}}}},
                 NormalUpdateRequest {{Steps::UnsubscribeListener {0u}}, {{0u, {}}, {1u, {bDown}}}},
             });
}

// TODO: More tests...

END_SUITE
