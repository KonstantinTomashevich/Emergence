#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Input/Input.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/Test/UpdateResultCheck.hpp>
#include <Celerity/UI/UI.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test::UpdateResultCheck
{
class ResultChecker final : public TaskExecutorBase<ResultChecker>
{
public:
    ResultChecker (TaskConstructor &_constructor,
                   FrameInputAccumulator *_inputAccumulator,
                   Container::Vector<Frame> _frames) noexcept;

    void Execute () noexcept;

private:
    void CompareInputEvents () noexcept;

    void CompareInputActionHolders () noexcept;

    void ExecuteTasks () noexcept;

    FetchSequenceQuery fetchInputActionHolders;

    FetchValueQuery fetchButtonByNodeId;
    FetchValueQuery fetchCheckboxByNodeId;
    FetchValueQuery fetchContainerByNodeId;
    FetchValueQuery fetchImageByNodeId;
    FetchValueQuery fetchInputByNodeId;
    FetchValueQuery fetchLabelByNodeId;
    FetchValueQuery fetchWindowByNodeId;

    FrameInputAccumulator *inputAccumulator;
    uint64_t currentFrameIndex = 0u;
    Container::Vector<Frame> frames;
};

ResultChecker::ResultChecker (TaskConstructor &_constructor,
                              FrameInputAccumulator *_inputAccumulator,
                              Container::Vector<Frame> _frames) noexcept
    : fetchInputActionHolders (FETCH_SEQUENCE (InputActionHolder)),

      fetchButtonByNodeId (FETCH_VALUE_1F (ButtonControl, nodeId)),
      fetchCheckboxByNodeId (FETCH_VALUE_1F (CheckboxControl, nodeId)),
      fetchContainerByNodeId (FETCH_VALUE_1F (ContainerControl, nodeId)),
      fetchImageByNodeId (FETCH_VALUE_1F (ImageControl, nodeId)),
      fetchInputByNodeId (FETCH_VALUE_1F (InputControl, nodeId)),
      fetchLabelByNodeId (FETCH_VALUE_1F (LabelControl, nodeId)),
      fetchWindowByNodeId (FETCH_VALUE_1F (WindowControl, nodeId)),

      inputAccumulator (_inputAccumulator),
      frames (std::move (_frames))
{
    _constructor.DependOn (UI::Checkpoint::UPDATE_FINISHED);
    _constructor.MakeDependencyOf (Input::Checkpoint::ACTION_DISPATCH_STARTED);
    _constructor.MakeDependencyOf (UI::Checkpoint::RENDER_STARTED);
}

void ResultChecker::Execute () noexcept
{
    if (currentFrameIndex > frames.size ())
    {
        return;
    }

    CompareInputEvents ();
    CompareInputActionHolders ();
    ExecuteTasks ();
    ++currentFrameIndex;
}

void ResultChecker::CompareInputEvents () noexcept
{
    Container::Vector<InputEvent> extractedEvents;
    for (auto eventIterator = inputAccumulator->EventsBegin (); eventIterator != inputAccumulator->EventsEnd ();
         ++eventIterator)
    {
        extractedEvents.emplace_back (*eventIterator);
    }

    CHECK_EQUAL (extractedEvents.size (), frames[currentFrameIndex].expectedInputLeft.size ());
    for (const InputEvent &event : extractedEvents)
    {
        const size_t found = std::count (extractedEvents.begin (), extractedEvents.end (), event);
        const size_t expected = std::count (frames[currentFrameIndex].expectedInputLeft.begin (),
                                            frames[currentFrameIndex].expectedInputLeft.end (), event);
        CHECK_EQUAL (found, expected);
    }
}

void ResultChecker::CompareInputActionHolders () noexcept
{
    Container::Vector<InputActionHolder> extractedHolders;
    for (auto cursor = fetchInputActionHolders.Execute ();
         const auto *holder = static_cast<const InputActionHolder *> (*cursor); ++cursor)
    {
        extractedHolders.emplace_back (*holder);
    }

    CHECK_EQUAL (extractedHolders.size (), frames[currentFrameIndex].expectedInput.size ());
    for (const InputActionHolder &holder : extractedHolders)
    {
        const size_t found = std::count (extractedHolders.begin (), extractedHolders.end (), holder);
        const size_t expected = std::count (frames[currentFrameIndex].expectedInput.begin (),
                                            frames[currentFrameIndex].expectedInput.end (), holder);
        CHECK_EQUAL (found, expected);
    }
}

void ResultChecker::ExecuteTasks () noexcept
{
    for (const Task &task : frames[currentFrameIndex].tasks)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, Tasks::ExpectCheckboxValue>)
                {
                    LOG ("Checking that checkbox with id ", _task.nodeId, " is ",
                         _task.checked ? "checked" : "not checked", ".");

                    auto checkboxCursor = fetchCheckboxByNodeId.Execute (&_task.nodeId);
                    const auto *checkbox = static_cast<const CheckboxControl *> (*checkboxCursor);
                    REQUIRE (checkbox);
                    CHECK_EQUAL (checkbox->checked, _task.checked);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectInputIntValue>)
                {
                    LOG ("Checking that integer input with id ", _task.nodeId, " has value ", _task.value, ".");
                    auto inputCursor = fetchInputByNodeId.Execute (&_task.nodeId);
                    const auto *input = static_cast<const InputControl *> (*inputCursor);
                    REQUIRE (input);
                    REQUIRE (input->type == InputControlType::INT);
                    CHECK_EQUAL (input->intValue, _task.value);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectInputFloatValue>)
                {
                    LOG ("Checking that float input with id ", _task.nodeId, " has value ", _task.value, ".");
                    auto inputCursor = fetchInputByNodeId.Execute (&_task.nodeId);
                    const auto *input = static_cast<const InputControl *> (*inputCursor);
                    REQUIRE (input);
                    REQUIRE (input->type == InputControlType::FLOAT);
                    CHECK_EQUAL (input->floatValue, _task.value);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectInputStringValue>)
                {
                    LOG ("Checking that string input with id ", _task.nodeId, " has value \"", _task.utf8TextValue,
                         "\".");
                    auto inputCursor = fetchInputByNodeId.Execute (&_task.nodeId);
                    const auto *input = static_cast<const InputControl *> (*inputCursor);
                    REQUIRE (input);
                    REQUIRE (input->type == InputControlType::TEXT);
                    CHECK_EQUAL (strcmp (input->utf8TextValue.data (), _task.utf8TextValue.data ()), 0);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectWindowOpen>)
                {
                    LOG ("Checking that window with id ", _task.nodeId, " is ", _task.open ? "open" : "closed", ".");
                    auto windowCursor = fetchWindowByNodeId.Execute (&_task.nodeId);
                    const auto *window = static_cast<const WindowControl *> (*windowCursor);
                    REQUIRE (window);
                    CHECK_EQUAL (window->open, _task.open);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectWindowPosition>)
                {
                    LOG ("Checking that window with id ", _task.nodeId, " has (", _task.x, "; ", _task.y,
                         ") position.");

                    auto windowCursor = fetchWindowByNodeId.Execute (&_task.nodeId);
                    const auto *window = static_cast<const WindowControl *> (*windowCursor);
                    REQUIRE (window);
                    CHECK_EQUAL (window->x, _task.x);
                    CHECK_EQUAL (window->y, _task.y);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectWindowSize>)
                {
                    LOG ("Checking that window with id ", _task.nodeId, " has (", _task.width, "; ", _task.height,
                         ") size.");

                    auto windowCursor = fetchWindowByNodeId.Execute (&_task.nodeId);
                    const auto *window = static_cast<const WindowControl *> (*windowCursor);
                    REQUIRE (window);
                    CHECK_EQUAL (window->width, _task.width);
                    CHECK_EQUAL (window->height, _task.height);
                }
                else if constexpr (std::is_same_v<Type, Tasks::ExpectControlsRemoved>)
                {
                    LOG ("Checking that there is no control with id ", _task.nodeId, ".");
                    auto buttonCursor = fetchButtonByNodeId.Execute (&_task.nodeId);
                    CHECK (!*buttonCursor);

                    auto checkboxCursor = fetchCheckboxByNodeId.Execute (&_task.nodeId);
                    CHECK (!*checkboxCursor);

                    auto containerCursor = fetchContainerByNodeId.Execute (&_task.nodeId);
                    CHECK (!*containerCursor);

                    auto imageCursor = fetchImageByNodeId.Execute (&_task.nodeId);
                    CHECK (!*imageCursor);

                    auto inputCursor = fetchInputByNodeId.Execute (&_task.nodeId);
                    CHECK (!*inputCursor);

                    auto labelCursor = fetchLabelByNodeId.Execute (&_task.nodeId);
                    CHECK (!*labelCursor);

                    auto windowCursor = fetchWindowByNodeId.Execute (&_task.nodeId);
                    CHECK (!*windowCursor);
                }
            },
            task);
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        FrameInputAccumulator *_inputAccumulator,
                        Container::Vector<Frame> _frames) noexcept
{
    _pipelineBuilder.AddTask (Memory::UniqueString {"UpdateResultChecker"})
        .SetExecutor<ResultChecker> (_inputAccumulator, std::move (_frames));
}
} // namespace Emergence::Celerity::Test::UpdateResultCheck
