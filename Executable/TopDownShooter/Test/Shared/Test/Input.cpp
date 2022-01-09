#include <SyntaxSugar/MuteWarnings.hpp>

#include <variant>

BEGIN_MUTING_WARNINGS
#include <OgreInput.h>
END_MUTING_WARNINGS

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <Log/Log.hpp>

#include <Input/InputCollection.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/NormalInputMappingSingleton.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Test/Input.hpp>
#include <Test/SharedApplicationContext.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity;

using namespace Emergence::Container;
using namespace Emergence::Container::Literals;
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
};

struct UnsubscribeListener
{
    std::uintptr_t id = INVALID_OBJECT_ID;
};

struct UnsubscribeGroup
{
    Emergence::Memory::UniqueString id;
};

struct FireEvent
{
    OgreBites::Event event;
};
} // namespace Steps

using ConfiguratorStep = std::variant<Steps::CreateListener,
                                      Steps::DeleteListener,
                                      Steps::AddSubscription,
                                      Steps::UnsubscribeListener,
                                      Steps::UnsubscribeGroup,
                                      Steps::FireEvent>;

using FrameConfiguration = Emergence::Container::Vector<ConfiguratorStep>;

class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor, Emergence::Container::Vector<FrameConfiguration> _steps);

    void Execute ();

private:
    Emergence::Container::Vector<FrameConfiguration> steps;
    std::size_t framesConfigured = 0u;

    Emergence::Warehouse::InsertLongTermQuery createListener;
    Emergence::Warehouse::ModifyValueQuery modifyListenerById;
    Emergence::Warehouse::ModifySingletonQuery modifyInputMapping;
};

Configurator::Configurator (TaskConstructor &_constructor, Emergence::Container::Vector<FrameConfiguration> _steps)
    : steps (std::move (_steps)),
      createListener (_constructor.InsertLongTerm (InputListenerObject::Reflect ().mapping)),
      modifyListenerById (_constructor.ModifyValue (InputListenerObject::Reflect ().mapping,
                                                    {InputListenerObject::Reflect ().objectId})),
      modifyInputMapping (_constructor.ModifySingleton (NormalInputMappingSingleton::Reflect ().mapping))
{
    _constructor.MakeDependencyOf (Checkpoint::INPUT_DISPATCH_BEGIN);
}

void Configurator::Execute ()
{
    using namespace Emergence::Log;
    REQUIRE (framesConfigured < steps.size ());
    // TODO: In tests it's ok to create myriads of strings just for logging.
    //       But for runtime logging we need special stack allocator which is cleared every frame.
    GlobalLogger::Log (Level::INFO, "[Configurator] Sequence " + ToString (framesConfigured));

    for (const ConfiguratorStep &step : steps[framesConfigured])
    {
        std::visit (
            [this] (const auto &_step)
            {
                using Type = std::decay_t<decltype (_step)>;
                if constexpr (std::is_same_v<Type, Steps::CreateListener>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Create listener " + ToString (_step.id) + ".");

                    auto cursor = createListener.Execute ();
                    (new (++cursor) InputListenerObject ())->objectId = _step.id;
                }
                else if constexpr (std::is_same_v<Type, Steps::DeleteListener>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Delete listener " + ToString (_step.id) + ".");

                    auto cursor = modifyListenerById.Execute (&_step.id);
                    REQUIRE (*cursor != nullptr);
                    ~cursor;
                    REQUIRE (*cursor == nullptr);
                }
                else if constexpr (std::is_same_v<Type, Steps::AddSubscription>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Subscribe listener " +
                                                        ToString (_step.subscription.listenerId) + " to group \"" +
                                                        *_step.subscription.group + "\".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.subscriptions.EmplaceBack (_step.subscription);
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeListener>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Unsubscribe listener " + ToString (_step.id) + ".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.UnsubscribeListener (_step.id);
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeGroup>)
                {
                    GlobalLogger::Log (Level::INFO,
                                       "[Configurator] Unsubscribe all listeners from group \""_s + *_step.id + "\".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.UnsubscribeGroup (_step.id);
                }
                else if constexpr (std::is_same_v<Type, Steps::FireEvent>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Fire event with type " +
                                                        ToString (_step.event.key.type) + " for character " +
                                                        static_cast<char> (_step.event.key.keysym.sym) + ".");

                    SharedApplicationContext::Get ()->_fireInputEvent (_step.event, 0u);
                }
            },
            step);
    }

    ++framesConfigured;
}

void AddConfiguratorTask (PipelineBuilder &_pipelineBuilder, Emergence::Container::Vector<FrameConfiguration> _steps)
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("Configurator"_us);
    constructor.SetExecutor<Configurator> (std::move (_steps));
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
    _constructor.DependOn (Checkpoint::INPUT_DISPATCH_END);
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

void RunTest (const Emergence::Container::Vector<KeyboardActionTrigger> &_keyboardTriggers,
              const Emergence::Container::Vector<std::pair<FrameConfiguration, FrameExpectation>> &_frames)
{
    World world {"TestWorld"_us};
    PipelineBuilder pipelineBuilder {&world};

    // In test definitions it's much more convenient to read config->expectation pairs than two separate vectors,
    // therefore we accept composite vector as parameter and split it inside this helper.

    Emergence::Container::Vector<FrameConfiguration> steps;
    steps.reserve (_frames.size ());

    for (const auto &pair : _frames)
    {
        steps.emplace_back (pair.first);
    }

    Emergence::Container::Vector<FrameExpectation> expectations;
    expectations.reserve (_frames.size ());

    for (const auto &pair : _frames)
    {
        expectations.emplace_back (pair.second);
    }

    pipelineBuilder.Begin ("Test"_us);
    AddConfiguratorTask (pipelineBuilder, std::move (steps));
    AddValidatorTask (pipelineBuilder, std::move (expectations));

    // We test only normal update, because normal and fixed input collectors are
    // implemented through the same class: only used singletons are different.
    InputCollection::AddNormalUpdateTask (SharedApplicationContext::Get (), pipelineBuilder);
    AddAllCheckpoints (pipelineBuilder);
    Pipeline *update = pipelineBuilder.End (std::thread::hardware_concurrency ());

    {
        auto modifyInput = world.ModifySingletonExternally (NormalInputMappingSingleton::Reflect ().mapping);
        auto *input = static_cast<NormalInputMappingSingleton *> (*modifyInput.Execute ());

        for (const KeyboardActionTrigger &trigger : _keyboardTriggers)
        {
            input->inputMapping.keyboardTriggers.EmplaceBack (trigger);
        }
    }

    for (std::size_t index = 0u; index < _frames.size (); ++index)
    {
        update->Execute ();
    }
}

Steps::FireEvent KeyboardEvent (char _key, OgreBites::EventType _type)
{
    Steps::FireEvent step;
    step.event.key.type = static_cast<int> (_type);
    step.event.key.keysym = {static_cast<OgreBites::Keycode> (_key), 0u};
    step.event.key.repeat = 0u;
    return step;
}

BEGIN_SUITE (Input)

TEST_CASE (SubscriptionManagement)
{
    char aButton = 'a';
    char bButton = 'b';

    InputAction aDown {"ADown"_us, "A"_us};
    InputAction bDown {"BDown"_us, "B"_us};

    RunTest (
        {
            KeyboardActionTrigger {aDown, {{aButton, false}}},
            KeyboardActionTrigger {bDown, {{bButton, false}}},
        },
        {
            {{Steps::CreateListener {0u}, Steps::AddSubscription {{"A"_us, 0u}}, Steps::CreateListener {1u},
              Steps::AddSubscription {{"B"_us, 1u}}},
             {}},
            {{KeyboardEvent (aButton, OgreBites::KEYDOWN), KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown}}, {1u, {bDown}}}},
            {{Steps::AddSubscription {{"B"_us, 0u}}, KeyboardEvent (aButton, OgreBites::KEYDOWN),
              KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown, bDown}}, {1u, {bDown}}}},
            {{Steps::UnsubscribeGroup {"B"_us}, KeyboardEvent (aButton, OgreBites::KEYDOWN),
              KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown}}, {1u, {}}}},
            {{Steps::AddSubscription {{"B"_us, 0u}}, Steps::AddSubscription {{"B"_us, 1u}},
              KeyboardEvent (aButton, OgreBites::KEYDOWN), KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown, bDown}}, {1u, {bDown}}}},
            {{Steps::UnsubscribeListener {0u}, KeyboardEvent (aButton, OgreBites::KEYDOWN),
              KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {}}, {1u, {bDown}}}},
        });
}

TEST_CASE (KeyboardTriggers)
{
    char firstButton = 'q';
    char secondButton = 'w';

    InputAction firstDown {"FirstDown"_us, "Test"_us};
    InputAction firstJustPressed {"FirstJustPressed"_us, "Test"_us};

    InputAction bothDown {"BothDown"_us, "Test"_us};
    InputAction bothJustPressed {"BothJustPressed"_us, "Test"_us};
    InputAction firstJustPressedSecondDown {"FirstJustPressedSecondDown"_us, "Test"_us};

    RunTest (
        {
            KeyboardActionTrigger {firstDown, {{firstButton, false}}},
            KeyboardActionTrigger {firstJustPressed, {{firstButton, true}}},
            KeyboardActionTrigger {bothDown, {{firstButton, false}, {secondButton, false}}},
            KeyboardActionTrigger {bothJustPressed, {{firstButton, true}, {secondButton, true}}},
            KeyboardActionTrigger {firstJustPressedSecondDown, {{firstButton, true}, {secondButton, false}}},
        },
        {
            {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test"_us, 0u}}}, {}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN)}, {{0u, {firstDown, firstJustPressed}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN)}, {{0u, {firstDown}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYUP)}, {{0u, {}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN)}, {{0u, {firstDown, firstJustPressed}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN), KeyboardEvent (secondButton, OgreBites::KEYDOWN)},
             {{0u, {firstDown, bothDown}}}},
            {{KeyboardEvent (secondButton, OgreBites::KEYUP)}, {{0u, {firstDown}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYUP)}, {{0u, {}}}},
            {{KeyboardEvent (secondButton, OgreBites::KEYDOWN)}, {{0u, {}}}},
            {{KeyboardEvent (secondButton, OgreBites::KEYUP)}, {{0u, {}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN), KeyboardEvent (secondButton, OgreBites::KEYDOWN)},
             {{0u, {firstDown, firstJustPressed, bothDown, bothJustPressed, firstJustPressedSecondDown}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN), KeyboardEvent (secondButton, OgreBites::KEYDOWN)},
             {{0u, {firstDown, bothDown}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYUP), KeyboardEvent (secondButton, OgreBites::KEYDOWN)},
             {{0u, {}}}},
            {{KeyboardEvent (firstButton, OgreBites::KEYDOWN), KeyboardEvent (secondButton, OgreBites::KEYDOWN)},
             {{0u, {firstDown, firstJustPressed, bothDown, firstJustPressedSecondDown}}}},
        });
}

END_SUITE
