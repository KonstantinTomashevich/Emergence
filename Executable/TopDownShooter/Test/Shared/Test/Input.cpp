#include <unordered_map>
#include <variant>

#pragma warning(push, 0)
#include <OgreInput.h>
#pragma warning(pop)

#include <Log/Log.hpp>

#include <Input/InputCollection.hpp>
#include <Input/InputListenerObject.hpp>
#include <Input/NormalInputMappingSingleton.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Test/Input.hpp>
#include <Test/SharedApplicationContext.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Celerity;

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
    Emergence::String::ConstReference id;
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

using FrameConfiguration = std::vector<ConfiguratorStep>;

class Configurator final
{
public:
    Configurator (std::vector<FrameConfiguration> _steps, TaskConstructor &_constructor);

    void Execute ();

private:
    std::vector<FrameConfiguration> steps;
    std::size_t framesConfigured = 0u;

    Emergence::Warehouse::InsertLongTermQuery createListener;
    Emergence::Warehouse::ModifyValueQuery modifyListenerById;
    Emergence::Warehouse::ModifySingletonQuery modifyInputMapping;
};

Configurator::Configurator (std::vector<FrameConfiguration> _steps, TaskConstructor &_constructor)
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
    GlobalLogger::Log (Level::INFO, "[Configurator] Sequence " + std::to_string (framesConfigured));

    for (const ConfiguratorStep &step : steps[framesConfigured])
    {
        std::visit (
            [this] (const auto &_step)
            {
                using Type = std::decay_t<decltype (_step)>;
                if constexpr (std::is_same_v<Type, Steps::CreateListener>)
                {
                    GlobalLogger::Log (Level::INFO,
                                       "[Configurator] Create listener " + std::to_string (_step.id) + ".");

                    auto cursor = createListener.Execute ();
                    (new (++cursor) InputListenerObject ())->objectId = _step.id;
                }
                else if constexpr (std::is_same_v<Type, Steps::DeleteListener>)
                {
                    GlobalLogger::Log (Level::INFO,
                                       "[Configurator] Delete listener " + std::to_string (_step.id) + ".");

                    auto cursor = modifyListenerById.Execute (&_step.id);
                    REQUIRE (*cursor != nullptr);
                    ~cursor;
                    REQUIRE (*cursor == nullptr);
                }
                else if constexpr (std::is_same_v<Type, Steps::AddSubscription>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Subscribe listener " +
                                                        std::to_string (_step.subscription.listenerId) +
                                                        " to group \"" + _step.subscription.group.Value () + "\".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.subscriptions.EmplaceBack (_step.subscription);
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeListener>)
                {
                    GlobalLogger::Log (Level::INFO,
                                       "[Configurator] Unsubscribe listener " + std::to_string (_step.id) + ".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.UnsubscribeListener (_step.id);
                }
                else if constexpr (std::is_same_v<Type, Steps::UnsubscribeGroup>)
                {
                    GlobalLogger::Log (Level::INFO,
                                       std::string ("[Configurator] Unsubscribe all listeners from group \"") +
                                           _step.id.Value () + "\".");

                    auto cursor = modifyInputMapping.Execute ();
                    auto *singleton = static_cast<NormalInputMappingSingleton *> (*cursor);
                    singleton->inputMapping.UnsubscribeGroup (_step.id);
                }
                else if constexpr (std::is_same_v<Type, Steps::FireEvent>)
                {
                    GlobalLogger::Log (Level::INFO, "[Configurator] Fire event with type " +
                                                        std::to_string (_step.event.key.type) + " for character " +
                                                        static_cast<char> (_step.event.key.keysym.sym) + ".");

                    SharedApplicationContext::Get ()->_fireInputEvent (_step.event, 0u);
                }
            },
            step);
    }

    ++framesConfigured;
}

void AddConfiguratorTask (PipelineBuilder &_pipelineBuilder, std::vector<FrameConfiguration> _steps)
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("Configurator");
    constructor.SetExecutor (
        [state {Configurator {std::move (_steps), constructor}}] () mutable
        {
            state.Execute ();
        });
}

using FrameExpectation = std::vector<std::pair<std::int64_t, std::vector<InputAction>>>;

class Validator final
{
public:
    Validator (std::vector<FrameExpectation> _expectations, TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    std::size_t framesValidated = 0u;
    std::vector<FrameExpectation> expectations;

    Emergence::Warehouse::FetchValueQuery fetchListenerById;
};

Validator::Validator (std::vector<FrameExpectation> _expectations, TaskConstructor &_constructor) noexcept
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

void AddValidatorTask (PipelineBuilder &_pipelineBuilder, std::vector<FrameExpectation> _expectations)
{
    Emergence::Celerity::TaskConstructor constructor = _pipelineBuilder.AddTask ("Validator");
    constructor.SetExecutor (
        [state {Validator {std::move (_expectations), constructor}}] () mutable
        {
            state.Execute ();
        });
}

void RunTest (const std::vector<KeyboardActionTrigger> &_keyboardTriggers,
              const std::vector<std::pair<FrameConfiguration, FrameExpectation>> &_frames)
{
    World world {"TestWorld"};
    PipelineBuilder pipelineBuilder {&world};

    // In test definitions it's much more convenient to read config->expectation pairs than two separate vectors,
    // therefore we accept composite vector as parameter and split it inside this helper.

    std::vector<FrameConfiguration> steps;
    steps.reserve (_frames.size ());

    for (const auto &pair : _frames)
    {
        steps.emplace_back (pair.first);
    }

    std::vector<FrameExpectation> expectations;
    expectations.reserve (_frames.size ());

    for (const auto &pair : _frames)
    {
        expectations.emplace_back (pair.second);
    }

    pipelineBuilder.Begin ();
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

    InputAction aDown {"ADown", "A"};
    InputAction bDown {"BDown", "B"};

    RunTest (
        {
            KeyboardActionTrigger {aDown, {{aButton, false}}},
            KeyboardActionTrigger {bDown, {{bButton, false}}},
        },
        {
            {{Steps::CreateListener {0u}, Steps::AddSubscription {{"A", 0u}}, Steps::CreateListener {1u},
              Steps::AddSubscription {{"B", 1u}}},
             {}},
            {{KeyboardEvent (aButton, OgreBites::KEYDOWN), KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown}}, {1u, {bDown}}}},
            {{Steps::AddSubscription {{"B", 0u}}, KeyboardEvent (aButton, OgreBites::KEYDOWN),
              KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown, bDown}}, {1u, {bDown}}}},
            {{Steps::UnsubscribeGroup {"B"}, KeyboardEvent (aButton, OgreBites::KEYDOWN),
              KeyboardEvent (bButton, OgreBites::KEYDOWN)},
             {{0u, {aDown}}, {1u, {}}}},
            {{Steps::AddSubscription {{"B", 0u}}, Steps::AddSubscription {{"B", 1u}},
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

    InputAction firstDown {"FirstDown", "Test"};
    InputAction firstJustPressed {"FirstJustPressed", "Test"};

    InputAction bothDown {"BothDown", "Test"};
    InputAction bothJustPressed {"BothJustPressed", "Test"};
    InputAction firstJustPressedSecondDown {"FirstJustPressedSecondDown", "Test"};

    RunTest (
        {
            KeyboardActionTrigger {firstDown, {{firstButton, false}}},
            KeyboardActionTrigger {firstJustPressed, {{firstButton, true}}},
            KeyboardActionTrigger {bothDown, {{firstButton, false}, {secondButton, false}}},
            KeyboardActionTrigger {bothJustPressed, {{firstButton, true}, {secondButton, true}}},
            KeyboardActionTrigger {firstJustPressedSecondDown, {{firstButton, true}, {secondButton, false}}},
        },
        {
            {{Steps::CreateListener {0u}, Steps::AddSubscription {{"Test", 0u}}}, {}},
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
