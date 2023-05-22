#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Locale/Localization.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/UI/AssetUsage.hpp>
#include <Celerity/UI/Events.hpp>
#include <Celerity/UI/Test/ControlManagement.hpp>
#include <Celerity/UI/Test/Functional.hpp>
#include <Celerity/UI/Test/ResourceProviderHolder.hpp>
#include <Celerity/UI/Test/SDLContextHolder.hpp>
#include <Celerity/UI/Test/UpdateResultCheck.hpp>
#include <Celerity/UI/UI.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
bool FunctionalTestIncludeMarker () noexcept
{
    return true;
}

static constexpr KeyCode KEY_RETURN = 1u;
static constexpr KeyCode KEY_ESCAPE = 2u;

KeyCodeMapping GetKeyCodeMapping ()
{
    KeyCodeMapping mapping;
    memset (&mapping, 0u, sizeof (mapping));
    mapping.keyReturn = KEY_RETURN;
    mapping.keyEscape = KEY_ESCAPE;
    return mapping;
}

struct Frame
{
    Container::Vector<InputEvent> inputEvents;
    ControlManagement::Frame controlManagementFrame;
    UpdateResultCheck::Frame resultCheckFrame;
};

static void ExecuteScenario (const Container::Vector<Frame> &_frames)
{
    using namespace Memory::Literals;

    Container::Vector<ControlManagement::Frame> controlManagementFrames;
    Container::Vector<UpdateResultCheck::Frame> resultCheckFrames;

    for (const Frame &frame : _frames)
    {
        controlManagementFrames.emplace_back (frame.controlManagementFrame);
        resultCheckFrames.emplace_back (frame.resultCheckFrame);
    }

    World world {Emergence::Memory::UniqueString {"TestWorld"}, {{1.0f / 60.0f}}};
    FrameInputAccumulator inputAccumulator;
    AssetReferenceBindingList binding {GetAssetBindingAllocationGroup ()};
    AssetReferenceBindingEventMap assetReferenceBindingEventMap;
    GetUIAssetUsage (binding);

    {
        EventRegistrar registrar {&world};
        assetReferenceBindingEventMap = RegisterAssetEvents (registrar, binding);
        RegisterRenderFoundationEvents (registrar);
        RegisterUIEvents (registrar);
    }

    PipelineBuilder pipelineBuilder {world.GetRootView ()};
    pipelineBuilder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    AssetManagement::AddToNormalUpdate (pipelineBuilder, binding, assetReferenceBindingEventMap);
    ControlManagement::AddToNormalUpdate (pipelineBuilder, std::move (controlManagementFrames));
    Input::AddToNormalUpdate (pipelineBuilder, &inputAccumulator);
    MaterialManagement::AddToNormalUpdate (pipelineBuilder, &GetSharedResourceProvider (),
                                           assetReferenceBindingEventMap);
    RenderPipelineFoundation::AddToNormalUpdate (pipelineBuilder);
    TextureManagement::AddToNormalUpdate (pipelineBuilder, &GetSharedResourceProvider (),
                                          assetReferenceBindingEventMap);
    UI::AddToNormalUpdate (pipelineBuilder, &inputAccumulator, GetKeyCodeMapping ());
    UpdateResultCheck::AddToNormalUpdate (pipelineBuilder, &inputAccumulator, std::move (resultCheckFrames));

    // Mock localization and transform hierarchy cleanup checkpoints.
    pipelineBuilder.AddCheckpoint (Localization::Checkpoint::SYNC_STARTED);
    pipelineBuilder.AddCheckpoint (Localization::Checkpoint::SYNC_FINISHED);
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::STARTED);
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED);
    pipelineBuilder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);
    REQUIRE (pipelineBuilder.End ());

    for (const auto &frame : _frames)
    {
        ContextHolder::Frame ();
        for (const InputEvent &event : frame.inputEvents)
        {
            inputAccumulator.RecordEvent (event);
        }

        world.Update ();
        inputAccumulator.Clear ();
    }
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test::ControlManagement::Tasks;
using namespace Emergence::Celerity::Test::UpdateResultCheck::Tasks;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity;
using namespace Emergence::Memory::Literals;

static const InputAction DEFAULT_ON_CLOSE_ACTION {"TestGroup"_us, "WindowClosed"_us};

static const InputAction DEFAULT_BUTTON_CLICKED_ACTION {"TestGroup"_us, "ButtonClicked"_us};

static const InputAction DEFAULT_CHECKBOX_CHANGED_ACTION {"TestGroup"_us, "CheckboxChanged"_us};

static const InputAction DEFAULT_INPUT_CHANGED_ACTION {"TestGroup"_us, "InputChanged"_us};

static InputActionHolder CreateActionHolder (InputAction _action)
{
    InputActionHolder holder;
    holder.action = std::move (_action);
    holder.dispatchType = InputActionDispatchType::NORMAL;
    return holder;
}

static InputAction WithCheckboxState (InputAction _action, bool _state)
{
    _action.discrete[0u] = _state ? 1u : 0u;
    return _action;
}

static InputAction WithInputInt (InputAction _action, int32_t _input)
{
    _action.discrete[0u] = _input;
    return _action;
}

static InputAction WithInputFloat (InputAction _action, float _input)
{
    _action.real[0u] = _input;
    return _action;
}

static CreateViewport CreateDefaultViewport ()
{
    return {"UI"_us, 0u, 0u, 0u, WIDTH, HEIGHT, 0x000000FF, ""_us};
}

static CreateWindow CreateDefaultWindow ()
{
    return {
        0u,
        ""_us,
        "UI"_us,
        ""_us,
        "Hello, world!",
        true,
        true,
        true,
        true,
        true,
        false,
        true,
        ContainerControlLayout::VERTICAL,
        {0.0f, 0.0f},
        {0.0f, 0.0f},
        0u,
        0u,
        WIDTH,
        HEIGHT,
        {},
        {},
    };
}

static CreateWindow WithOnClose (CreateWindow _task)
{
    _task.onClosedAction = DEFAULT_ON_CLOSE_ACTION;
    _task.onClosedActionDispatch = InputActionDispatchType::NORMAL;
    return _task;
}

static CreateWindow NonCloseable (CreateWindow _task)
{
    _task.closable = false;
    return _task;
}

static CreateWindow NotMovable (CreateWindow _task)
{
    _task.movable = false;
    return _task;
}

static CreateWindow NotResizable (CreateWindow _task)
{
    _task.resizable = false;
    return _task;
}

static CreateWindow WithCustomSize (CreateWindow _task, uint32_t _width, uint32_t _height)
{
    _task.width = _width;
    _task.height = _height;
    return _task;
}

static CreateButton CreateDefaultButton ()
{
    return {
        1u, 0u, ""_us, ""_us, "Click me!", 100u, 50u, DEFAULT_BUTTON_CLICKED_ACTION, InputActionDispatchType::NORMAL};
}

static CreateCheckbox CreateDefaultCheckbox ()
{
    return {1u, 0u, ""_us, ""_us, "Check me!", false, DEFAULT_CHECKBOX_CHANGED_ACTION, InputActionDispatchType::NORMAL};
}

static CreateInput CreateDefaultInput ()
{
    CreateInput task;
    task.nodeId = 1u;
    task.parentId = 0u;
    task.type = InputControlType::INT;
    task.label = "Sample input";
    task.onChangedAction = DEFAULT_INPUT_CHANGED_ACTION;
    task.onChangedActionDispatch = InputActionDispatchType::NORMAL;
    return task;
}

static CreateInput AsIntInput (CreateInput _task, int32_t _value)
{
    _task.type = InputControlType::INT;
    _task.intValue = _value;
    return _task;
}

static CreateInput AsFloatInput (CreateInput _task, float _value)
{
    _task.type = InputControlType::FLOAT;
    _task.floatValue = _value;
    return _task;
}

static CreateInput AsTextInput (CreateInput _task, const Emergence::Container::Utf8String &_value)
{
    _task.type = InputControlType::TEXT;
    REQUIRE (_value.size () < InputControl::MAX_TEXT_LENGTH);
    strcpy (_task.utf8TextValue.data (), _value.c_str ());
    return _task;
}

BEGIN_SUITE (Functional)

TEST_CASE (WindowClose)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                WithOnClose (CreateDefaultWindow ()),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 20u, 5u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 20u, 5u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {
                    CreateActionHolder (DEFAULT_ON_CLOSE_ACTION),
                },
                {},
                {
                    ExpectWindowOpen {0u, false},
                },
            },
        },
    });
}

TEST_CASE (WindowCloseBlocked)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                NonCloseable (CreateDefaultWindow ()),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 20u, 5u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 20u, 5u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectWindowOpen {0u, true},
                },
            },
        },
    });
}

TEST_CASE (WindowMove)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH / 2u, 5u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseMotionEvent {WIDTH / 2u, 5u, WIDTH / 2u + 100u, 55u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH / 2u + 100u, 55u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectWindowPosition {0u, 100u, 50u},
                },
            },
        },
    });
}

TEST_CASE (WindowMoveBlocked)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                NotMovable (CreateDefaultWindow ()),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH / 2u, 5u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseMotionEvent {WIDTH / 2u, 5u, WIDTH / 2u + 100u, 55u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH / 2u + 100u, 55u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectWindowPosition {0u, 0u, 0u},
                },
            },
        },
    });
}

TEST_CASE (WindowResize)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 10u, HEIGHT - 10u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseMotionEvent {WIDTH - 10u, HEIGHT - 10u, WIDTH - 215u, HEIGHT - 115u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 215u, HEIGHT - 115u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectWindowSize {0u, WIDTH - 205u, HEIGHT - 105u},
                },
            },
        },
    });
}

TEST_CASE (WindowResizeBlocked)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                NotResizable (CreateDefaultWindow ()),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 10u, HEIGHT - 10u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseMotionEvent {WIDTH - 10u, HEIGHT - 10u, WIDTH - 215u, HEIGHT - 115u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {WIDTH - 215u, HEIGHT - 115u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectWindowSize {0u, WIDTH, HEIGHT},
                },
            },
        },
    });
}

TEST_CASE (NoInputBlockedOnControlMiss)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                WithCustomSize (CreateDefaultWindow (), 200u, 200u),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {250u, 250u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {
                    {0u, MouseButtonEvent {250u, 250u, MouseButton::LEFT, KeyState::DOWN, 1u}},
                },
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {250u, 250u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {
                    {0u, MouseButtonEvent {250u, 250u, MouseButton::LEFT, KeyState::UP, 1u}},
                },
                {},
            },
        },
    });
}

TEST_CASE (ButtonClick)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                CreateDefaultButton (),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {
                    CreateActionHolder (DEFAULT_BUTTON_CLICKED_ACTION),
                },
                {},
                {},
            },
        },
    });
}

TEST_CASE (ButtonClickMissed)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                CreateDefaultButton (),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {100u, 100u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {100u, 100u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
    });
}

TEST_CASE (Checkbox)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                CreateDefaultCheckbox (),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {
                    CreateActionHolder (WithCheckboxState (DEFAULT_CHECKBOX_CHANGED_ACTION, true)),
                },
                {},
                {
                    ExpectCheckboxValue {1u, true},
                },
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {
                    CreateActionHolder (WithCheckboxState (DEFAULT_CHECKBOX_CHANGED_ACTION, false)),
                },
                {},
                {
                    ExpectCheckboxValue {1u, false},
                },
            },
        },
    });
}

TEST_CASE (InputIntCorrect)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsIntInput (CreateDefaultInput (), 12),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"4567"}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputInt (DEFAULT_INPUT_CHANGED_ACTION, 4567)),
                },
                {},
                {
                    ExpectInputIntValue {1u, 4567},
                },
            },
        },
    });
}

TEST_CASE (InputIntIncorrect)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsIntInput (CreateDefaultInput (), 12),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"abc"}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectInputIntValue {1u, 12},
                },
            },
        },
        {
            {
                {0u, KeyboardEvent {KEY_RETURN, KEY_RETURN, 0u, KeyState::DOWN}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectInputIntValue {1u, 12},
                },
            },
        },
    });
}

TEST_CASE (InputIntCancel)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsIntInput (CreateDefaultInput (), 12),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"4567"}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputInt (DEFAULT_INPUT_CHANGED_ACTION, 4567)),
                },
                {},
                {
                    ExpectInputIntValue {1u, 4567},
                },
            },
        },
        {
            {
                {0u, KeyboardEvent {KEY_ESCAPE, KEY_ESCAPE, 0u, KeyState::DOWN}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputInt (DEFAULT_INPUT_CHANGED_ACTION, 12)),
                },
                {},
                {
                    ExpectInputIntValue {1u, 12},
                },
            },
        },
    });
}

TEST_CASE (InputFloatCorrect)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsFloatInput (CreateDefaultInput (), 49.987f),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"13.984361"}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputFloat (DEFAULT_INPUT_CHANGED_ACTION, 13.984361f)),
                },
                {},
                {
                    ExpectInputFloatValue {1u, 13.984361f},
                },
            },
        },
    });
}

TEST_CASE (InputFloatIncorrect)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsFloatInput (CreateDefaultInput (), 49.987f),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"abc"}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectInputFloatValue {1u, 49.987f},
                },
            },
        },
        {
            {
                {0u, KeyboardEvent {KEY_RETURN, KEY_RETURN, 0u, KeyState::DOWN}},
            },
            {},
            {
                {},
                {},
                {
                    ExpectInputFloatValue {1u, 49.987f},
                },
            },
        },
    });
}

TEST_CASE (InputFloatCancel)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsFloatInput (CreateDefaultInput (), 49.987f),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"13.984361"}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputFloat (DEFAULT_INPUT_CHANGED_ACTION, 13.984361f)),
                },
                {},
                {
                    ExpectInputFloatValue {1u, 13.984361f},
                },
            },
        },
        {
            {
                {0u, KeyboardEvent {KEY_ESCAPE, KEY_ESCAPE, 0u, KeyState::DOWN}},
            },
            {},
            {
                {
                    CreateActionHolder (WithInputFloat (DEFAULT_INPUT_CHANGED_ACTION, 49.987f)),
                },
                {},
                {
                    ExpectInputFloatValue {1u, 49.987f},
                },
            },
        },
    });
}

TEST_CASE (InputText)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsTextInput (CreateDefaultInput (), "Hello, world!"),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"New text value, yep!"}},
            },
            {},
            {
                {
                    CreateActionHolder (DEFAULT_INPUT_CHANGED_ACTION),
                },
                {},
                {
                    ExpectInputStringValue {1u, {"New text value, yep!"}},
                },
            },
        },
    });
}

TEST_CASE (InputTextCancel)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                AsTextInput (CreateDefaultInput (), "Hello, world!"),
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::DOWN, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, MouseButtonEvent {30u, 30u, MouseButton::LEFT, KeyState::UP, 1u}},
            },
            {},
            {
                {},
                {},
                {},
            },
        },
        {
            {
                {0u, TextInputEvent {"New text value, yep!"}},
            },
            {},
            {
                {
                    CreateActionHolder (DEFAULT_INPUT_CHANGED_ACTION),
                },
                {},
                {
                    ExpectInputStringValue {1u, {"New text value, yep!"}},
                },
            },
        },
        {
            {
                {0u, KeyboardEvent {KEY_ESCAPE, KEY_ESCAPE, 0u, KeyState::DOWN}},
            },
            {},
            {
                {
                    CreateActionHolder (DEFAULT_INPUT_CHANGED_ACTION),
                },
                {},
                {
                    ExpectInputStringValue {1u, {"Hello, world!"}},
                },
            },
        },
    });
}

TEST_CASE (HierarchyRemoval)
{
    ExecuteScenario ({
        {
            {

            },
            {
                CreateDefaultViewport (),
                CreateDefaultWindow (),
                CreateButton {1u, 0u, ""_us, ""_us, "Click me!", 100u, 50u, DEFAULT_BUTTON_CLICKED_ACTION,
                              InputActionDispatchType::NORMAL},
                CreateButton {2u, 0u, ""_us, ""_us, "Click me too!", 100u, 50u, DEFAULT_BUTTON_CLICKED_ACTION,
                              InputActionDispatchType::NORMAL},
                CreateContainer {3u, 0u, ""_us, ContainerControlType::PANEL, ContainerControlLayout::HORIZONTAL, 0u, 0u,
                                 true, ""_us, "", ""_us, ""},
                CreateCheckbox {4u, 3u, ""_us, ""_us, "Check me!", false, DEFAULT_CHECKBOX_CHANGED_ACTION,
                                InputActionDispatchType::NORMAL},
                CreateCheckbox {5u, 3u, ""_us, ""_us, "Check me too!", false, DEFAULT_CHECKBOX_CHANGED_ACTION,
                                InputActionDispatchType::NORMAL},
                CreateLabel {6u, 3u, ""_us, ""_us, "Hello, world!"},
                CreateImage {7u, 3u, ""_us, 100u, 100u, "T_Texture"_us, {{0.0f, 0.0f}, {1.0f, 1.0f}}},
            },
            {
                {},
                {},
                {},
            },
        },
        {
            {

            },
            {RemoveControl {0u}},
            {
                {},
                {},
                {},
            },
        },
        {
            {},
            {},
            {
                {},
                {},
                {
                    ExpectControlsRemoved {0u},
                    ExpectControlsRemoved {1u},
                    ExpectControlsRemoved {2u},
                    ExpectControlsRemoved {3u},
                    ExpectControlsRemoved {4u},
                    ExpectControlsRemoved {5u},
                    ExpectControlsRemoved {6u},
                    ExpectControlsRemoved {7u},
                },
            },
        },
    });
}

END_SUITE
