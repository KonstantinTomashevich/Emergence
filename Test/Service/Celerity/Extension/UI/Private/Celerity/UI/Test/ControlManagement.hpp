#pragma once

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/InputControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test::ControlManagement
{
namespace Tasks
{
struct CreateViewport
{
    Memory::UniqueString viewportName;
    std::uint16_t sortIndex = 0u;
    std::uint32_t x = 0u;
    std::uint32_t y = 0u;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;
    std::uint32_t clearColor = 0x000000FF;
    Memory::UniqueString defaultStyleId;
};

struct CreateButton final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;

    Memory::UniqueString labelKey;
    Container::Utf8String label;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    InputAction onClickAction;
    InputActionDispatchType onClickActionDispatch = InputActionDispatchType::NORMAL;
};

struct CreateCheckbox final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;

    Memory::UniqueString labelKey;
    Container::Utf8String label;
    bool checked = false;

    InputAction onChangedAction;
    InputActionDispatchType onChangedActionDispatch = InputActionDispatchType::NORMAL;
};

struct CreateContainer final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;

    ContainerControlType type = ContainerControlType::PANEL;
    ContainerControlLayout layout = ContainerControlLayout::VERTICAL;

    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    bool border = false;

    Memory::UniqueString labelKey;
    Container::Utf8String label;
    Memory::UniqueString previewKey;
    Container::Utf8String preview;
};

struct CreateImage final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;

    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    Memory::UniqueString textureId;
    Math::AxisAlignedBox2d uv;
};

struct CreateInput final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;

    InputControlType type = InputControlType::TEXT;
    Memory::UniqueString labelKey;
    Container::Utf8String label;

    InputAction onChangedAction;
    InputActionDispatchType onChangedActionDispatch = InputActionDispatchType::NORMAL;

    union
    {
        std::array<char, Celerity::InputControl::MAX_TEXT_LENGTH> utf8TextValue;
        int32_t intValue;
        float floatValue;
    };
};

struct CreateLabel final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    UniqueId parentId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;
    Memory::UniqueString labelKey;
    Container::Utf8String label;
};

struct CreateWindow final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    Memory::UniqueString styleId;
    Memory::UniqueString viewportName;
    Memory::UniqueString titleKey;
    Container::Utf8String title;

    bool closable = true;
    bool minimizable = true;
    bool resizable = true;
    bool movable = true;
    bool hasTitleBar = true;
    bool pack = false;

    bool open = true;
    ContainerControlLayout layout = ContainerControlLayout::VERTICAL;

    Math::Vector2f anchor = Math::Vector2f::ZERO;
    Math::Vector2f pivot = Math::Vector2f::ZERO;

    int32_t x = 0;
    int32_t y = 0;
    std::uint32_t width = 0u;
    std::uint32_t height = 0u;

    InputAction onClosedAction;
    InputActionDispatchType onClosedActionDispatch = InputActionDispatchType::NORMAL;
};

struct CreateStyleColorProperty final
{
    Memory::UniqueString styleId;
    UIStyleColorPropertyName property = UIStyleColorPropertyName::TEXT;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float alpha = 1.0f;
};

struct CreateStyleFloatProperty final
{
    Memory::UniqueString styleId;
    UIStyleFloatPropertyName property = UIStyleFloatPropertyName::ALPHA;
    float value = 0.0f;
};

struct CreateStyleFloatPairProperty final
{
    Memory::UniqueString styleId;
    UIStyleFloatPairPropertyName property = UIStyleFloatPairPropertyName::WINDOW_PADDING;
    float x = 0.0f;
    float y = 0.0f;
};

struct CreateStyleFontProperty final
{
    Memory::UniqueString styleId;
    Memory::UniqueString fontId;
};

struct RemoveControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
};
} // namespace Tasks

using Task = Container::Variant<Tasks::CreateViewport,
                                Tasks::CreateButton,
                                Tasks::CreateCheckbox,
                                Tasks::CreateContainer,
                                Tasks::CreateImage,
                                Tasks::CreateInput,
                                Tasks::CreateLabel,
                                Tasks::CreateWindow,
                                Tasks::CreateStyleColorProperty,
                                Tasks::CreateStyleFloatProperty,
                                Tasks::CreateStyleFloatPairProperty,
                                Tasks::CreateStyleFontProperty,
                                Tasks::RemoveControl>;

using Frame = Container::Vector<Task>;

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<Frame> _frames) noexcept;
} // namespace Emergence::Celerity::Test::ControlManagement
