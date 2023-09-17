#pragma once

#include <CelerityUIModelApi.hpp>

#include <Celerity/Input/InputAction.hpp>
#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/UI/ContainerControl.hpp>

#include <Container/String.hpp>

#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents window: special instance of ContainerControl that is always a toplevel node of hierarchy.
/// \invariant Windows and only windows can be controls of toplevel nodes.
struct CelerityUIModelApi WindowControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Name of the viewport in which this window is rendered.
    Memory::UniqueString viewportName;

    /// \brief Key to LocalizedString that is used as window title if any.
    Memory::UniqueString titleKey;

    /// \brief Window title. Used instead of ::titleKey if present.
    Container::Utf8String title;

    /// \brief Whether this window can be closed.
    bool closable = true;

    /// \brief Whether this window can be minimized.
    bool minimizable = true;

    /// \brief Whether this window can be resized.
    bool resizable = true;

    /// \brief Whether this window can be moved around.
    bool movable = true;

    /// \brief Whether this window title bar is visible.
    bool hasTitleBar = true;

    /// \brief If true, window will be automatically resized to fit all the elements.
    bool pack = false;

    /// \brief Whether window is open right now.
    bool open = true;

    /// \brief Defines how children are positioned inside window.
    ContainerControlLayout layout = ContainerControlLayout::VERTICAL;

    /// \brief Point of the viewport to which this window position is attached. Defined as ratio to viewport size.
    Math::Vector2f anchor = Math::Vector2f::ZERO;

    /// \brief Point of the window that is attached to defined viewport translation. Defined as ratio to window size.
    Math::Vector2f pivot = Math::Vector2f::ZERO;

    /// \brief Window translation horizontal component, relative to ::anchor and ::pivot.
    int32_t x = 0;

    /// \brief Window translation vertical component, relative to ::anchor and ::pivot.
    int32_t y = 0;

    /// \brief Target window width.
    std::uint32_t width = 0u;

    /// \brief Target window height.
    std::uint32_t height = 0u;

    /// \brief Input action that will be sent in InputActionHolder on window close.
    InputAction onClosedAction;

    /// \brief Dispatch type for ::onClosedAction.
    InputActionDispatchType onClosedActionDispatch = InputActionDispatchType::NORMAL;

    struct CelerityUIModelApi Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId viewportName;
        StandardLayout::FieldId titleKey;
        StandardLayout::FieldId title;

        StandardLayout::FieldId closable;
        StandardLayout::FieldId minimizable;
        StandardLayout::FieldId resizable;
        StandardLayout::FieldId movable;
        StandardLayout::FieldId hasTitleBar;
        StandardLayout::FieldId pack;

        StandardLayout::FieldId open;
        StandardLayout::FieldId layout;

        StandardLayout::FieldId anchor;
        StandardLayout::FieldId pivot;

        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;

        StandardLayout::FieldId onClosedAction;
        StandardLayout::FieldId onClosedActionDispatch;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
