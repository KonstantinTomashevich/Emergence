#pragma once

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/Input/InputActionHolder.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/UI/InputControl.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test::UpdateResultCheck
{
namespace Tasks
{
struct ExpectCheckboxValue final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    bool checked = false;
};

struct ExpectInputIntValue final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    int32_t value = 0;
};

struct ExpectInputFloatValue final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    float value = 0.0f;
};

struct ExpectInputStringValue final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    std::array<char, InputControl::MAX_TEXT_LENGTH> utf8TextValue;
};

struct ExpectWindowOpen final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    bool open = false;
};

struct ExpectWindowPosition final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    int32_t x = 0;
    int32_t y = 0;
};

struct ExpectWindowSize final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    uint32_t width = 0u;
    uint32_t height = 0u;
};

struct ExpectControlsRemoved final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
};
} // namespace Tasks

using Task = Container::Variant<Tasks::ExpectCheckboxValue,
                                Tasks::ExpectInputIntValue,
                                Tasks::ExpectInputFloatValue,
                                Tasks::ExpectInputStringValue,
                                Tasks::ExpectWindowOpen,
                                Tasks::ExpectWindowPosition,
                                Tasks::ExpectWindowSize,
                                Tasks::ExpectControlsRemoved>;

struct Frame final
{
    Container::Vector<InputActionHolder> expectedInput;
    Container::Vector<InputEvent> expectedInputLeft;
    Container::Vector<Task> tasks;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        FrameInputAccumulator *_inputAccumulator,
                        Container::Vector<Frame> _frames) noexcept;
} // namespace Emergence::Celerity::Test::UpdateResultCheck
