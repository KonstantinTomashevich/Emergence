#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionHolder.hpp>
#include <Celerity/Input/Keyboard.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/InputControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/UIAssetPin.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIProcessing.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/UI/UIStyle.hpp>
#include <Celerity/UI/UI.hpp>
#include <Celerity/UI/WindowControl.hpp>

#include <imgui.h>

#include <Log/Log.hpp>

#include <Memory/Stack.hpp>

namespace Emergence::Celerity::UIProcessing
{
using namespace Memory::Literals;

struct NodeInfo final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    uint64_t sortIndex = 0u;
};

static_assert (std::is_trivially_destructible_v<NodeInfo>);

class NodeOrderingStack final
{
public:
    class Sequence final
    {
    public:
        Sequence (NodeOrderingStack *_stack) noexcept;

        Sequence (const Sequence &_context) = delete;

        Sequence (Sequence &&_context) = delete;

        ~Sequence () noexcept;

        void Add (UniqueId _nodeId, uint64_t _sortIndex) noexcept;

        void Sort () noexcept;

        [[nodiscard]] const NodeInfo *Begin () const noexcept;

        [[nodiscard]] const NodeInfo *End () const noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Sequence);

    private:
        NodeOrderingStack *stack = nullptr;
        const NodeInfo *sequenceBegin = nullptr;
        const NodeInfo *sequenceEnd = nullptr;
    };

private:
    friend class Sequence;

    static constexpr size_t STACK_SIZE = sizeof (NodeInfo) * 1024u;

    Memory::Stack stack {Memory::Profiler::AllocationGroup {"NodeOrderingStack"_us}, STACK_SIZE};
};

NodeOrderingStack::Sequence::Sequence (NodeOrderingStack *_stack) noexcept
    : stack (_stack),
      sequenceBegin (static_cast<const NodeInfo *> (stack->stack.Head ())),
      sequenceEnd (static_cast<const NodeInfo *> (stack->stack.Head ()))
{
}

NodeOrderingStack::Sequence::~Sequence () noexcept
{
    EMERGENCE_ASSERT (stack->stack.Head () == sequenceEnd);
    stack->stack.Release (sequenceBegin);
}

void NodeOrderingStack::Sequence::Add (UniqueId _nodeId, uint64_t _sortIndex) noexcept
{
    EMERGENCE_ASSERT (stack->stack.Head () == sequenceEnd);
    new (stack->stack.Acquire (sizeof (NodeInfo), alignof (NodeInfo))) NodeInfo {_nodeId, _sortIndex};
    sequenceEnd = static_cast<const NodeInfo *> (stack->stack.Head ());
}

void NodeOrderingStack::Sequence::Sort () noexcept
{
    // We need to const cast in order to allow element swapping.
    std::sort (const_cast<NodeInfo *> (sequenceBegin), const_cast<NodeInfo *> (sequenceEnd),
               [] (const NodeInfo &_first, const NodeInfo &_second)
               {
                   return _first.sortIndex < _second.sortIndex;
               });
}

const NodeInfo *NodeOrderingStack::Sequence::Begin () const noexcept
{
    return sequenceBegin;
}

const NodeInfo *NodeOrderingStack::Sequence::End () const noexcept
{
    return sequenceEnd;
}

const NodeInfo *begin (const NodeOrderingStack::Sequence &_sequence) noexcept
{
    return _sequence.Begin ();
}

const NodeInfo *end (const NodeOrderingStack::Sequence &_sequence) noexcept
{
    return _sequence.End ();
}

class StyleApplier final
{
public:
    class Context final
    {
    public:
        Context (StyleApplier *_applier, Memory::UniqueString _styleId) noexcept;

        Context (const Context &_context) = delete;

        Context (Context &&_context) = delete;

        ~Context () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Context);

    private:
        uint8_t colorCount = 0u;
        uint8_t varCount = 0u;
        uint8_t fontCount = 0u;
    };

    StyleApplier (TaskConstructor &_constructor) noexcept;

private:
    friend class Context;

    FetchValueQuery fetchColorPropertyByNodeId;
    FetchValueQuery fetchFloatPropertyByNodeId;
    FetchValueQuery fetchFloatPairPropertyByNodeId;
    FetchValueQuery fetchFontPropertyByNodeId;
    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchFontByAssetId;
};

StyleApplier::Context::Context (StyleApplier *_applier, Memory::UniqueString _styleId) noexcept
{
    if (!*_styleId)
    {
        return;
    }

    for (auto cursor = _applier->fetchColorPropertyByNodeId.Execute (&_styleId);
         const auto *property = static_cast<const UIStyleColorProperty *> (*cursor); ++cursor)
    {
        ++colorCount;
        const ImVec4 color {property->red, property->green, property->blue, property->alpha};

        switch (property->property)
        {
        case UIStyleColorPropertyName::TEXT:
            ImGui::PushStyleColor (ImGuiCol_Text, color);
            break;

        case UIStyleColorPropertyName::DISABLED_TEXT:
            ImGui::PushStyleColor (ImGuiCol_TextDisabled, color);
            break;

        case UIStyleColorPropertyName::WINDOW_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_WindowBg, color);
            break;

        case UIStyleColorPropertyName::CONTAINER_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_ChildBg, color);
            break;

        case UIStyleColorPropertyName::POPUP_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_PopupBg, color);
            break;

        case UIStyleColorPropertyName::BORDER:
            ImGui::PushStyleColor (ImGuiCol_Border, color);
            break;

        case UIStyleColorPropertyName::BORDER_SHADOW:
            ImGui::PushStyleColor (ImGuiCol_BorderShadow, color);
            break;

        case UIStyleColorPropertyName::TITLE_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_TitleBg, color);
            break;

        case UIStyleColorPropertyName::TITLE_BACKGROUND_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_TitleBgActive, color);
            break;

        case UIStyleColorPropertyName::TITLE_BACKGROUND_COLLAPSED:
            ImGui::PushStyleColor (ImGuiCol_TitleBgCollapsed, color);
            break;

        case UIStyleColorPropertyName::MENUBAR_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_MenuBarBg, color);
            break;

        case UIStyleColorPropertyName::SCROLLBAR_BACKGROUND:
            ImGui::PushStyleColor (ImGuiCol_ScrollbarBg, color);
            break;

        case UIStyleColorPropertyName::SCROLLBAR_GRAB:
            ImGui::PushStyleColor (ImGuiCol_ScrollbarGrab, color);
            break;

        case UIStyleColorPropertyName::SCROLLBAR_GRAB_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_ScrollbarGrabHovered, color);
            break;

        case UIStyleColorPropertyName::SCROLLBAR_GRAB_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_ScrollbarGrabActive, color);
            break;

        case UIStyleColorPropertyName::CHECK_MARK:
            ImGui::PushStyleColor (ImGuiCol_CheckMark, color);
            break;

        case UIStyleColorPropertyName::SLIDER_GRAB:
            ImGui::PushStyleColor (ImGuiCol_SliderGrab, color);
            break;

        case UIStyleColorPropertyName::BUTTON:
            ImGui::PushStyleColor (ImGuiCol_Button, color);
            break;

        case UIStyleColorPropertyName::BUTTON_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_ButtonHovered, color);
            break;

        case UIStyleColorPropertyName::BUTTON_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_ButtonActive, color);
            break;

        case UIStyleColorPropertyName::HEADER:
            ImGui::PushStyleColor (ImGuiCol_Header, color);
            break;

        case UIStyleColorPropertyName::HEADER_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_HeaderHovered, color);
            break;

        case UIStyleColorPropertyName::HEADER_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_HeaderActive, color);
            break;

        case UIStyleColorPropertyName::SEPARATOR:
            ImGui::PushStyleColor (ImGuiCol_Separator, color);
            break;

        case UIStyleColorPropertyName::SEPARATOR_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_SeparatorHovered, color);
            break;

        case UIStyleColorPropertyName::SEPARATOR_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_SeparatorActive, color);
            break;

        case UIStyleColorPropertyName::RESIZE_GRIP:
            ImGui::PushStyleColor (ImGuiCol_ResizeGrip, color);
            break;

        case UIStyleColorPropertyName::RESIZE_GRIP_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_ResizeGripHovered, color);
            break;

        case UIStyleColorPropertyName::RESIZE_GRIP_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_ResizeGripActive, color);
            break;

        case UIStyleColorPropertyName::TAB:
            ImGui::PushStyleColor (ImGuiCol_Tab, color);
            break;

        case UIStyleColorPropertyName::TAB_HOVERED:
            ImGui::PushStyleColor (ImGuiCol_TabHovered, color);
            break;

        case UIStyleColorPropertyName::TAB_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_TabActive, color);
            break;

        case UIStyleColorPropertyName::TAB_UNFOCUSED:
            ImGui::PushStyleColor (ImGuiCol_TabUnfocused, color);
            break;

        case UIStyleColorPropertyName::TAB_UNFOCUSED_ACTIVE:
            ImGui::PushStyleColor (ImGuiCol_TabUnfocusedActive, color);
            break;
        }
    }

    for (auto cursor = _applier->fetchFloatPropertyByNodeId.Execute (&_styleId);
         const auto *property = static_cast<const UIStyleFloatProperty *> (*cursor); ++cursor)
    {
        ++varCount;
        switch (property->property)
        {
        case UIStyleFloatPropertyName::ALPHA:
            ImGui::PushStyleVar (ImGuiStyleVar_Alpha, property->value);
            break;

        case UIStyleFloatPropertyName::DISABLED_ALPHA:
            ImGui::PushStyleVar (ImGuiStyleVar_DisabledAlpha, property->value);
            break;

        case UIStyleFloatPropertyName::WINDOW_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_WindowRounding, property->value);
            break;

        case UIStyleFloatPropertyName::WINDOW_BORDER_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_WindowBorderSize, property->value);
            break;

        case UIStyleFloatPropertyName::CONTAINER_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_ChildRounding, property->value);
            break;

        case UIStyleFloatPropertyName::CONTAINER_BORDER_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_ChildBorderSize, property->value);
            break;

        case UIStyleFloatPropertyName::POPUP_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_PopupRounding, property->value);
            break;

        case UIStyleFloatPropertyName::POPUP_BORDER_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_PopupBorderSize, property->value);
            break;

        case UIStyleFloatPropertyName::INDENT_SPACING:
            ImGui::PushStyleVar (ImGuiStyleVar_IndentSpacing, property->value);
            break;

        case UIStyleFloatPropertyName::SCROLLBAR_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_ScrollbarSize, property->value);
            break;

        case UIStyleFloatPropertyName::SCROLLBAR_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_ScrollbarRounding, property->value);
            break;

        case UIStyleFloatPropertyName::GRAB_MIN_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_GrabMinSize, property->value);
            break;

        case UIStyleFloatPropertyName::GRAB_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_GrabRounding, property->value);
            break;

        case UIStyleFloatPropertyName::TAB_ROUNDING:
            ImGui::PushStyleVar (ImGuiStyleVar_TabRounding, property->value);
            break;
        }
    }

    for (auto cursor = _applier->fetchFloatPairPropertyByNodeId.Execute (&_styleId);
         const auto *property = static_cast<const UIStyleFloatPairProperty *> (*cursor); ++cursor)
    {
        ++varCount;
        switch (property->property)
        {
        case UIStyleFloatPairPropertyName::WINDOW_PADDING:
            ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::WINDOW_MIN_SIZE:
            ImGui::PushStyleVar (ImGuiStyleVar_WindowMinSize, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::WINDOW_TITLE_ALIGN:
            ImGui::PushStyleVar (ImGuiStyleVar_WindowTitleAlign, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::ITEM_SPACING:
            ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::ITEM_INNER_SPACING:
            ImGui::PushStyleVar (ImGuiStyleVar_ItemInnerSpacing, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::BUTTON_TEXT_ALIGN:
            ImGui::PushStyleVar (ImGuiStyleVar_ButtonTextAlign, {property->x, property->y});
            break;

        case UIStyleFloatPairPropertyName::SELECTABLE_TEXT_ALIGN:
            ImGui::PushStyleVar (ImGuiStyleVar_SelectableTextAlign, {property->x, property->y});
            break;
        }
    }

    for (auto cursor = _applier->fetchFontPropertyByNodeId.Execute (&_styleId);
         const auto *property = static_cast<const UIStyleFontProperty *> (*cursor); ++cursor)
    {
        auto assetCursor = _applier->fetchAssetById.Execute (&property->fontId);
        const auto *asset = static_cast<const Asset *> (*assetCursor);

        if (!asset || asset->state != AssetState::READY)
        {
            continue;
        }

        auto fontCursor = _applier->fetchFontByAssetId.Execute (&property->fontId);
        const auto *font = static_cast<const Font *> (*fontCursor);

        EMERGENCE_ASSERT (font);
        EMERGENCE_ASSERT (font->nativeHandle);
        EMERGENCE_ASSERT (static_cast<const ImFontAtlas *> (font->nativeHandle)->Fonts.size () == 1u);

        ImGui::PushFont (static_cast<const ImFontAtlas *> (font->nativeHandle)->Fonts[0u]);
        ++fontCount;
    }
}

StyleApplier::Context::~Context () noexcept
{
    if (colorCount)
    {
        ImGui::PopStyleColor (static_cast<int> (colorCount));
    }

    if (varCount)
    {
        ImGui::PopStyleVar (static_cast<int> (varCount));
    }

    for (uint32_t index = 0u; index < fontCount; ++index)
    {
        ImGui::PopFont ();
    }
}

StyleApplier::StyleApplier (TaskConstructor &_constructor) noexcept
    : fetchColorPropertyByNodeId (FETCH_VALUE_1F (UIStyleColorProperty, styleId)),
      fetchFloatPropertyByNodeId (FETCH_VALUE_1F (UIStyleFloatProperty, styleId)),
      fetchFloatPairPropertyByNodeId (FETCH_VALUE_1F (UIStyleFloatPairProperty, styleId)),
      fetchFontPropertyByNodeId (FETCH_VALUE_1F (UIStyleFontProperty, styleId)),
      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchFontByAssetId (FETCH_VALUE_1F (Font, assetId))
{
}

class UIProcessor final : public TaskExecutorBase<UIProcessor>
{
public:
    UIProcessor (TaskConstructor &_constructor,
                 FrameInputAccumulator *_inputAccumulator,
                 const KeyCodeMapping &_keyCodeMapping) noexcept;

    void Execute ();

private:
    static void *CreateContext () noexcept;

    void SubmitInput (const Viewport *_viewport) noexcept;

    void ConsumeInput () noexcept;

    void ProcessNode (const UINode *_node) noexcept;

    void ProcessControl (const ButtonControl *_control) noexcept;

    void ProcessControl (CheckboxControl *_control) noexcept;

    void ProcessControl (const ContainerControl *_control) noexcept;

    void ProcessControl (const ImageControl *_control) noexcept;

    void ProcessControl (InputControl *_control) noexcept;

    static void ProcessControl (const LabelControl *_control) noexcept;

    void ProcessControl (WindowControl *_control) noexcept;

    void ProcessChildControls (const NodeOrderingStack::Sequence &_sequence, ContainerControlLayout _layout) noexcept;

    FetchSingletonQuery fetchTime;
    EditAscendingRangeQuery editRenderPasses;
    FetchValueQuery fetchViewportByName;

    FetchValueQuery fetchNodeByNodeId;
    FetchValueQuery fetchNodeByParentId;
    FetchValueQuery fetchWindowControlByViewport;

    FetchValueQuery fetchButtonControlByNodeId;
    EditValueQuery editCheckboxControlByNodeId;
    FetchValueQuery fetchContainerControlByNodeId;
    FetchValueQuery fetchImageControlByNodeId;
    EditValueQuery editInputControlByNodeId;
    FetchValueQuery fetchLabelControlByNodeId;
    EditValueQuery editWindowControlByNodeId;

    InsertShortTermQuery insertInputActionHolder;

    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchTextureByAssetId;

    FrameInputAccumulator *inputAccumulator;
    NodeOrderingStack nodeOrderingStack;
    StyleApplier styleApplier;

    Container::HashMap<KeyCode, ImGuiKey> keyMap {Memory::Profiler::AllocationGroup {"KeyMap"_us}};
};

UIProcessor::UIProcessor (TaskConstructor &_constructor,
                          FrameInputAccumulator *_inputAccumulator,
                          const KeyCodeMapping &_keyCodeMapping) noexcept
    : fetchTime (FETCH_SINGLETON (TimeSingleton)),
      editRenderPasses (EDIT_ASCENDING_RANGE (UIRenderPass, name)),
      fetchViewportByName (FETCH_VALUE_1F (Viewport, name)),

      fetchNodeByNodeId (FETCH_VALUE_1F (UINode, nodeId)),
      fetchNodeByParentId (FETCH_VALUE_1F (UINode, parentId)),
      fetchWindowControlByViewport (FETCH_VALUE_1F (WindowControl, viewportName)),

      fetchButtonControlByNodeId (FETCH_VALUE_1F (ButtonControl, nodeId)),
      editCheckboxControlByNodeId (EDIT_VALUE_1F (CheckboxControl, nodeId)),
      fetchContainerControlByNodeId (FETCH_VALUE_1F (ContainerControl, nodeId)),
      fetchImageControlByNodeId (FETCH_VALUE_1F (ImageControl, nodeId)),
      editInputControlByNodeId (EDIT_VALUE_1F (InputControl, nodeId)),
      fetchLabelControlByNodeId (FETCH_VALUE_1F (LabelControl, nodeId)),
      editWindowControlByNodeId (EDIT_VALUE_1F (WindowControl, nodeId)),

      insertInputActionHolder (INSERT_SHORT_TERM (InputActionHolder)),

      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchTextureByAssetId (FETCH_VALUE_1F (Texture, assetId)),

      inputAccumulator (_inputAccumulator),
      styleApplier (_constructor)
{
    _constructor.DependOn (UI::Checkpoint::UPDATE_STARTED);
    _constructor.MakeDependencyOf (UI::Checkpoint::UPDATE_FINISHED);

    // We produce input actions and would like to dispatch them right away.
    _constructor.MakeDependencyOf (Input::Checkpoint::ACTION_DISPATCH_STARTED);

    keyMap[_keyCodeMapping.keyEscape] = ImGuiKey_Escape;
    keyMap[_keyCodeMapping.keyReturn] = ImGuiKey_Enter;
    keyMap[_keyCodeMapping.keyBackspace] = ImGuiKey_Backspace;
    keyMap[_keyCodeMapping.keyTab] = ImGuiKey_Tab;
    keyMap[_keyCodeMapping.keyExclaim] = ImGuiKey_1;
    keyMap[_keyCodeMapping.keyQuoteDouble] = ImGuiKey_Apostrophe;
    keyMap[_keyCodeMapping.keySpace] = ImGuiKey_Space;
    keyMap[_keyCodeMapping.keyHash] = ImGuiKey_3;
    keyMap[_keyCodeMapping.keyPercent] = ImGuiKey_5;
    keyMap[_keyCodeMapping.keyDollar] = ImGuiKey_4;
    keyMap[_keyCodeMapping.keyAmpersand] = ImGuiKey_7;
    keyMap[_keyCodeMapping.keyQuote] = ImGuiKey_Apostrophe;
    keyMap[_keyCodeMapping.keyLeftParenthesis] = ImGuiKey_9;
    keyMap[_keyCodeMapping.keyRightParenthesis] = ImGuiKey_0;
    keyMap[_keyCodeMapping.keyAsterisk] = ImGuiKey_8;
    keyMap[_keyCodeMapping.keyPlus] = ImGuiKey_Equal;
    keyMap[_keyCodeMapping.keyComma] = ImGuiKey_Comma;
    keyMap[_keyCodeMapping.keyMinus] = ImGuiKey_Minus;
    keyMap[_keyCodeMapping.keyPeriod] = ImGuiKey_Period;
    keyMap[_keyCodeMapping.keySlash] = ImGuiKey_Slash;
    keyMap[_keyCodeMapping.key0] = ImGuiKey_0;
    keyMap[_keyCodeMapping.key1] = ImGuiKey_1;
    keyMap[_keyCodeMapping.key2] = ImGuiKey_2;
    keyMap[_keyCodeMapping.key3] = ImGuiKey_3;
    keyMap[_keyCodeMapping.key4] = ImGuiKey_4;
    keyMap[_keyCodeMapping.key5] = ImGuiKey_5;
    keyMap[_keyCodeMapping.key6] = ImGuiKey_6;
    keyMap[_keyCodeMapping.key7] = ImGuiKey_7;
    keyMap[_keyCodeMapping.key8] = ImGuiKey_8;
    keyMap[_keyCodeMapping.key9] = ImGuiKey_9;
    keyMap[_keyCodeMapping.keyColon] = ImGuiKey_Semicolon;
    keyMap[_keyCodeMapping.keySemicolon] = ImGuiKey_Semicolon;
    keyMap[_keyCodeMapping.keyLess] = ImGuiKey_Comma;
    keyMap[_keyCodeMapping.keyEquals] = ImGuiKey_Equal;
    keyMap[_keyCodeMapping.keyGreater] = ImGuiKey_Period;
    keyMap[_keyCodeMapping.keyQuestion] = ImGuiKey_Slash;
    keyMap[_keyCodeMapping.keyAt] = ImGuiKey_2;

    keyMap[_keyCodeMapping.keyLeftBracket] = ImGuiKey_LeftBracket;
    keyMap[_keyCodeMapping.keyBackSlash] = ImGuiKey_Backslash;
    keyMap[_keyCodeMapping.keyRightBracket] = ImGuiKey_RightBracket;
    keyMap[_keyCodeMapping.keyCaret] = ImGuiKey_6;
    keyMap[_keyCodeMapping.keyUnderscore] = ImGuiKey_Minus;
    keyMap[_keyCodeMapping.keyBackQuote] = ImGuiKey_GraveAccent;
    keyMap[_keyCodeMapping.keyA] = ImGuiKey_A;
    keyMap[_keyCodeMapping.keyB] = ImGuiKey_B;
    keyMap[_keyCodeMapping.keyC] = ImGuiKey_C;
    keyMap[_keyCodeMapping.keyD] = ImGuiKey_D;
    keyMap[_keyCodeMapping.keyE] = ImGuiKey_E;
    keyMap[_keyCodeMapping.keyF] = ImGuiKey_F;
    keyMap[_keyCodeMapping.keyG] = ImGuiKey_G;
    keyMap[_keyCodeMapping.keyH] = ImGuiKey_H;
    keyMap[_keyCodeMapping.keyI] = ImGuiKey_I;
    keyMap[_keyCodeMapping.keyJ] = ImGuiKey_J;
    keyMap[_keyCodeMapping.keyK] = ImGuiKey_K;
    keyMap[_keyCodeMapping.keyL] = ImGuiKey_L;
    keyMap[_keyCodeMapping.keyM] = ImGuiKey_M;
    keyMap[_keyCodeMapping.keyN] = ImGuiKey_N;
    keyMap[_keyCodeMapping.keyO] = ImGuiKey_O;
    keyMap[_keyCodeMapping.keyP] = ImGuiKey_P;
    keyMap[_keyCodeMapping.keyQ] = ImGuiKey_Q;
    keyMap[_keyCodeMapping.keyR] = ImGuiKey_R;
    keyMap[_keyCodeMapping.keyS] = ImGuiKey_S;
    keyMap[_keyCodeMapping.keyT] = ImGuiKey_T;
    keyMap[_keyCodeMapping.keyU] = ImGuiKey_U;
    keyMap[_keyCodeMapping.keyV] = ImGuiKey_V;
    keyMap[_keyCodeMapping.keyW] = ImGuiKey_W;
    keyMap[_keyCodeMapping.keyX] = ImGuiKey_X;
    keyMap[_keyCodeMapping.keyY] = ImGuiKey_Y;
    keyMap[_keyCodeMapping.keyZ] = ImGuiKey_Z;

    keyMap[_keyCodeMapping.keyCapsLock] = ImGuiKey_CapsLock;

    keyMap[_keyCodeMapping.keyF1] = ImGuiKey_F1;
    keyMap[_keyCodeMapping.keyF2] = ImGuiKey_F2;
    keyMap[_keyCodeMapping.keyF3] = ImGuiKey_F3;
    keyMap[_keyCodeMapping.keyF4] = ImGuiKey_F4;
    keyMap[_keyCodeMapping.keyF5] = ImGuiKey_F5;
    keyMap[_keyCodeMapping.keyF6] = ImGuiKey_F6;
    keyMap[_keyCodeMapping.keyF7] = ImGuiKey_F7;
    keyMap[_keyCodeMapping.keyF8] = ImGuiKey_F8;
    keyMap[_keyCodeMapping.keyF9] = ImGuiKey_F9;
    keyMap[_keyCodeMapping.keyF10] = ImGuiKey_F10;
    keyMap[_keyCodeMapping.keyF11] = ImGuiKey_F11;
    keyMap[_keyCodeMapping.keyF12] = ImGuiKey_F12;

    keyMap[_keyCodeMapping.keyPrintScreen] = ImGuiKey_PrintScreen;
    keyMap[_keyCodeMapping.keyScrollLock] = ImGuiKey_ScrollLock;
    keyMap[_keyCodeMapping.keyPause] = ImGuiKey_Pause;
    keyMap[_keyCodeMapping.keyInsert] = ImGuiKey_Insert;
    keyMap[_keyCodeMapping.keyHome] = ImGuiKey_Home;
    keyMap[_keyCodeMapping.keyPageUp] = ImGuiKey_PageUp;
    keyMap[_keyCodeMapping.keyDelete] = ImGuiKey_Delete;
    keyMap[_keyCodeMapping.keyEnd] = ImGuiKey_End;
    keyMap[_keyCodeMapping.keyPageDown] = ImGuiKey_PageDown;
    keyMap[_keyCodeMapping.keyRight] = ImGuiKey_RightArrow;
    keyMap[_keyCodeMapping.keyLeft] = ImGuiKey_LeftArrow;
    keyMap[_keyCodeMapping.keyDown] = ImGuiKey_DownArrow;
    keyMap[_keyCodeMapping.keyUp] = ImGuiKey_UpArrow;

    keyMap[_keyCodeMapping.keyNumLockClear] = ImGuiKey_NumLock;
    keyMap[_keyCodeMapping.keyPadDivide] = ImGuiKey_KeypadDivide;
    keyMap[_keyCodeMapping.keyPadMultiply] = ImGuiKey_KeypadMultiply;
    keyMap[_keyCodeMapping.keyPadMinus] = ImGuiKey_KeypadSubtract;
    keyMap[_keyCodeMapping.keyPadPlus] = ImGuiKey_KeypadAdd;
    keyMap[_keyCodeMapping.keyPadEnter] = ImGuiKey_KeypadEnter;
    keyMap[_keyCodeMapping.keyPad1] = ImGuiKey_Keypad1;
    keyMap[_keyCodeMapping.keyPad2] = ImGuiKey_Keypad2;
    keyMap[_keyCodeMapping.keyPad3] = ImGuiKey_Keypad3;
    keyMap[_keyCodeMapping.keyPad4] = ImGuiKey_Keypad4;
    keyMap[_keyCodeMapping.keyPad5] = ImGuiKey_Keypad5;
    keyMap[_keyCodeMapping.keyPad6] = ImGuiKey_Keypad6;
    keyMap[_keyCodeMapping.keyPad7] = ImGuiKey_Keypad7;
    keyMap[_keyCodeMapping.keyPad8] = ImGuiKey_Keypad8;
    keyMap[_keyCodeMapping.keyPad9] = ImGuiKey_Keypad9;
    keyMap[_keyCodeMapping.keyPad0] = ImGuiKey_Keypad0;
    keyMap[_keyCodeMapping.keyPadPeriod] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyApplication] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPower] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadEquals] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF13] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF14] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF15] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF16] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF17] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF18] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF19] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF20] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF21] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF22] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF23] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF24] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyExecute] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyHelp] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyMenu] = ImGuiKey_Menu;
    keyMap[_keyCodeMapping.keySelect] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyStop] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAgain] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyUndo] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCut] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCopy] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPaste] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyFind] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyMute] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyVolumeUp] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyVolumeDown] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadComma] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadEqualsAs400] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyAltErase] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keySystemRequest] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCancel] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyClear] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPrior] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyReturn2] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keySeparator] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyOut] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyOperation] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyClearAgain] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCarousel] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyExSel] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyPad00] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPad000] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyThousandsSeparator] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyDecimalSeparator] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCurrencyUnit] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCurrencySubUnit] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadLeftParenthesis] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadRightParenthesis] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadLeftBrace] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadRightBrace] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadTab] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadBackspace] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadA] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadB] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadC] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadD] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadE] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadF] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadXOR] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadPower] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadPercent] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadLess] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadGreater] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadAmpersand] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadDoubleAmpersand] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadVerticalBar] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadDoubleVerticalBar] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadColon] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadHash] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadSpace] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadAt] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadExclaim] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemStore] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemRecall] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemClear] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemAdd] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemSubtract] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemMultiply] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadMemDivide] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadPlusMinus] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadClear] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadClearEntry] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadBinary] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadOctal] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadDecimal] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyPadHexadecimal] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyLeftControl] = ImGuiKey_LeftCtrl;
    keyMap[_keyCodeMapping.keyLeftShift] = ImGuiKey_LeftShift;
    keyMap[_keyCodeMapping.keyLeftAlt] = ImGuiKey_LeftAlt;
    keyMap[_keyCodeMapping.keyLeftGui] = ImGuiKey_LeftSuper;
    keyMap[_keyCodeMapping.keyRightControl] = ImGuiKey_RightCtrl;
    keyMap[_keyCodeMapping.keyRightShift] = ImGuiKey_RightShift;
    keyMap[_keyCodeMapping.keyRightAlt] = ImGuiKey_RightAlt;
    keyMap[_keyCodeMapping.keyRightGui] = ImGuiKey_RightSuper;

    keyMap[_keyCodeMapping.keyMode] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyAudioNext] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAudioPrev] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAudioStop] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAudioPlay] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAudioMute] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyMediaSelect] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyWww] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyMail] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyCalculator] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyComputer] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlSearch] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlHome] = ImGuiKey_Home;
    keyMap[_keyCodeMapping.keyApplicationControlBack] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlForward] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlStop] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlRefresh] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApplicationControlBookmarks] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyBrightnessDown] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyBrightnessUp] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyDisplaySwitch] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyKeyboardIlluminationToggle] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyKeyboardIlluminationDown] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyKeyboardIlluminationUp] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyEject] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keySleep] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApp1] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyApp2] = ImGuiKey_None;

    keyMap[_keyCodeMapping.keyAudioRewind] = ImGuiKey_None;
    keyMap[_keyCodeMapping.keyAudioFastForward] = ImGuiKey_None;
}

void UIProcessor::Execute ()
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const TimeSingleton *> (*timeCursor);

    for (auto renderPassCursor = editRenderPasses.Execute (nullptr, nullptr);
         auto *renderPass = static_cast<UIRenderPass *> (*renderPassCursor); ++renderPassCursor)
    {
        auto viewportCursor = fetchViewportByName.Execute (&renderPass->name);
        const auto *viewport = static_cast<const Viewport *> (*viewportCursor);

        if (!viewport)
        {
            continue;
        }

        if (!renderPass->nativeContext)
        {
            renderPass->nativeContext = CreateContext ();
        }

        ImGui::SetCurrentContext (static_cast<ImGuiContext *> (renderPass->nativeContext));
        ImGuiIO &io = ImGui::GetIO ();
        io.DisplaySize = {static_cast<float> (viewport->width), static_cast<float> (viewport->height)};
        io.DisplayFramebufferScale = {1.0f, 1.0f};
        io.DeltaTime = time->realNormalDurationS;
        SubmitInput (viewport);
        ImGui::NewFrame ();

        const StyleApplier::Context styleContext {&styleApplier, renderPass->defaultStyleId};
        NodeOrderingStack::Sequence orderingSequence {&nodeOrderingStack};

        for (auto windowCursor = fetchWindowControlByViewport.Execute (&viewport->name);
             const auto *window = static_cast<const WindowControl *> (*windowCursor); ++windowCursor)
        {
            auto nodeCursor = fetchNodeByNodeId.Execute (&window->nodeId);
            const auto *node = static_cast<const UINode *> (*nodeCursor);
            EMERGENCE_ASSERT (node);
            EMERGENCE_ASSERT (node->parentId == INVALID_UNIQUE_ID);
            orderingSequence.Add (node->nodeId, node->sortIndex);
        }

        orderingSequence.Sort ();
        for (const NodeInfo &info : orderingSequence)
        {
            auto nodeCursor = fetchNodeByNodeId.Execute (&info.nodeId);
            const auto *node = static_cast<const UINode *> (*nodeCursor);
            ProcessNode (node);
        }

        ConsumeInput ();
    }
}

void *UIProcessor::CreateContext () noexcept
{
    ImGuiContext *context = ImGui::CreateContext ();
    ImGuiIO &io = ImGui::GetIO ();

    io.BackendPlatformName = "Emergence::Celerity::UI";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // TODO: Add clipboard implementation using SDL2?
    return context;
}

void UIProcessor::SubmitInput (const Viewport *_viewport) noexcept
{
    ImGuiIO &io = ImGui::GetIO ();
    auto isInViewport = [_viewport] (int32_t _x, int32_t _y)
    {
        return _x >= static_cast<int32_t> (_viewport->x) && _y >= static_cast<int32_t> (_viewport->y) &&
               _x < static_cast<int32_t> (_viewport->x + _viewport->width) &&
               _y < static_cast<int32_t> (_viewport->y + _viewport->height);
    };

    for (auto iterator = inputAccumulator->EventsBegin (); iterator != inputAccumulator->EventsEnd (); ++iterator)
    {
        const InputEvent &event = *iterator;
        switch (event.type)
        {
        case InputEventType::KEYBOARD:
            EMERGENCE_ASSERT (keyMap.contains (event.keyboard.keyCode));
            io.AddKeyEvent (keyMap.at (event.keyboard.keyCode), event.keyboard.keyState == KeyState::DOWN);
            break;

        case InputEventType::MOUSE_BUTTON:
            if (isInViewport (event.mouseButton.mouseX, event.mouseButton.mouseY))
            {
                switch (event.mouseButton.button)
                {
                case MouseButton::LEFT:
                    io.AddMouseButtonEvent (ImGuiMouseButton_Left, event.mouseButton.state == KeyState::DOWN);
                    break;

                case MouseButton::MIDDLE:
                    io.AddMouseButtonEvent (ImGuiMouseButton_Middle, event.mouseButton.state == KeyState::DOWN);
                    break;

                case MouseButton::RIGHT:
                    io.AddMouseButtonEvent (ImGuiMouseButton_Right, event.mouseButton.state == KeyState::DOWN);
                    break;

                case MouseButton::X1:
                case MouseButton::X2:
                    // Unsupported by ImGUI.
                    break;
                }
            }

            break;

        case InputEventType::MOUSE_MOTION:
            if (isInViewport (event.mouseMotion.newX, event.mouseMotion.newY))
            {
                io.AddMousePosEvent (static_cast<float> (event.mouseMotion.newX - static_cast<int32_t> (_viewport->x)),
                                     static_cast<float> (event.mouseMotion.newY - static_cast<int32_t> (_viewport->y)));
            }

            break;

        case InputEventType::MOUSE_WHEEL:
            io.AddMouseWheelEvent (event.mouseWheel.x, event.mouseWheel.y);
            break;
        }
    }
}

void UIProcessor::ConsumeInput () noexcept
{
    ImGuiIO &io = ImGui::GetIO ();
    auto inputIterator = inputAccumulator->EventsBegin ();

    while (inputIterator != inputAccumulator->EventsEnd ())
    {
        bool consume = false;
        switch ((*inputIterator).type)
        {
        case InputEventType::KEYBOARD:
            consume = io.WantCaptureKeyboard;
            break;

        case InputEventType::MOUSE_BUTTON:
        case InputEventType::MOUSE_MOTION:
        case InputEventType::MOUSE_WHEEL:
            consume = io.WantCaptureMouse;
            break;
        }

        if (consume)
        {
            ~inputIterator;
        }
        else
        {
            ++inputIterator;
        }
    }
}

void UIProcessor::ProcessNode (const UINode *_node) noexcept
{
    const StyleApplier::Context styleContext {&styleApplier, _node->styleId};
    ImGui::PushID (static_cast<int> (_node->nodeId));

    if (auto buttonCursor = fetchButtonControlByNodeId.Execute (&_node->nodeId);
        const auto *button = static_cast<const ButtonControl *> (*buttonCursor))
    {
        ProcessControl (button);
    }
    else if (auto checkboxCursor = editCheckboxControlByNodeId.Execute (&_node->nodeId);
             auto *checkbox = static_cast<CheckboxControl *> (*checkboxCursor))
    {
        ProcessControl (checkbox);
    }
    else if (auto containerCursor = fetchContainerControlByNodeId.Execute (&_node->nodeId);
             const auto *container = static_cast<const ContainerControl *> (*containerCursor))
    {
        ProcessControl (container);
    }
    else if (auto imageCursor = fetchImageControlByNodeId.Execute (&_node->nodeId);
             const auto *image = static_cast<const ImageControl *> (*imageCursor))
    {
        ProcessControl (image);
    }
    else if (auto inputCursor = editInputControlByNodeId.Execute (&_node->nodeId);
             auto *input = static_cast<InputControl *> (*inputCursor))
    {
        ProcessControl (input);
    }
    else if (auto labelCursor = fetchLabelControlByNodeId.Execute (&_node->nodeId);
             const auto *label = static_cast<const LabelControl *> (*labelCursor))
    {
        ProcessControl (label);
    }
    else if (auto windowCursor = editWindowControlByNodeId.Execute (&_node->nodeId);
             auto *window = static_cast<WindowControl *> (*windowCursor))
    {
        ProcessControl (window);
    }
    else
    {
        EMERGENCE_LOG (WARNING, "UI: Node ", _node->nodeId, " has no control attached to it!");
    }

    ImGui::PopID ();
}

void UIProcessor::ProcessControl (const ButtonControl *_control) noexcept
{
    if (ImGui::Button (_control->label.c_str (),
                       {static_cast<float> (_control->width), static_cast<float> (_control->height)}))
    {
        auto holderCursor = insertInputActionHolder.Execute ();
        auto *holder = static_cast<InputActionHolder *> (++holderCursor);
        holder->action = _control->onClickAction;
        holder->dispatchType = _control->onClickActionDispatch;
    }
}

void UIProcessor::ProcessControl (CheckboxControl *_control) noexcept
{
    if (ImGui::Checkbox (_control->label.c_str (), &_control->checked))
    {
        auto holderCursor = insertInputActionHolder.Execute ();
        auto *holder = static_cast<InputActionHolder *> (++holderCursor);
        holder->action = _control->onChangedAction;
        holder->action.discrete[0u] = _control->checked ? 1u : 0u;
        holder->dispatchType = _control->onChangedActionDispatch;
    }
}

void UIProcessor::ProcessControl (const ContainerControl *_control) noexcept
{
    NodeOrderingStack::Sequence orderingSequence {&nodeOrderingStack};
    for (auto nodeCursor = fetchNodeByParentId.Execute (&_control->nodeId);
         const auto *node = static_cast<const UINode *> (*nodeCursor); ++nodeCursor)
    {
        orderingSequence.Add (node->nodeId, node->sortIndex);
    }

    orderingSequence.Sort ();
    switch (_control->type)
    {
    case ContainerControlType::PANEL:
        ImGui::BeginChild (static_cast<int> (_control->nodeId),
                           {static_cast<float> (_control->width), static_cast<float> (_control->height)},
                           _control->border);
        ProcessChildControls (orderingSequence, _control->layout);
        ImGui::EndChild ();
        break;

    case ContainerControlType::COLLAPSING_PANEL:
        if (ImGui::CollapsingHeader (_control->label.c_str ()))
        {
            ProcessChildControls (orderingSequence, _control->layout);
        }

        break;

    case ContainerControlType::COMBO_PANEL:
        if (ImGui::BeginCombo (_control->label.c_str (), _control->preview.c_str ()))
        {
            ProcessChildControls (orderingSequence, _control->layout);
            ImGui::EndCombo ();
        }

        break;
    }
}

void UIProcessor::ProcessControl (const ImageControl *_control) noexcept
{
    if (auto assetCursor = fetchAssetById.Execute (&_control->textureId);
        !*assetCursor || static_cast<const Asset *> (*assetCursor)->state != AssetState::READY)
    {
        return;
    }

    auto textureCursor = fetchTextureByAssetId.Execute (&_control->textureId);
    const auto *texture = static_cast<const Texture *> (*textureCursor);
    EMERGENCE_ASSERT (texture);

    const Render::Backend::TextureId textureId = texture->texture.GetId ();
    const ImTextureID imGUITextureId = *reinterpret_cast<const ImTextureID *> (&textureId);

    ImGui::Image (imGUITextureId, {static_cast<float> (_control->width), static_cast<float> (_control->height)},
                  {_control->uv.min.x, _control->uv.min.y}, {_control->uv.max.x, _control->uv.max.y});
}

void UIProcessor::ProcessControl (InputControl *_control) noexcept
{
    switch (_control->type)
    {
    case InputControlType::TEXT:
        if (ImGui::InputText (_control->label.c_str (), _control->utf8TextValue.data (),
                              _control->utf8TextValue.size () - 1u))
        {
            auto holderCursor = insertInputActionHolder.Execute ();
            auto *holder = static_cast<InputActionHolder *> (++holderCursor);
            holder->action = _control->onChangedAction;
            holder->dispatchType = _control->onChangedActionDispatch;
        }

        break;

    case InputControlType::INT:
    {
        int valueHolder = static_cast<int> (_control->intValue);
        if (ImGui::InputInt (_control->label.c_str (), &valueHolder))
        {
            auto holderCursor = insertInputActionHolder.Execute ();
            auto *holder = static_cast<InputActionHolder *> (++holderCursor);
            holder->action = _control->onChangedAction;
            holder->action.discrete[0u] = static_cast<int32_t> (_control->intValue);
            holder->dispatchType = _control->onChangedActionDispatch;
        }

        _control->intValue = static_cast<int32_t> (valueHolder);
        break;
    }

    case InputControlType::FLOAT:
        if (ImGui::InputFloat (_control->label.c_str (), &_control->floatValue))
        {
            auto holderCursor = insertInputActionHolder.Execute ();
            auto *holder = static_cast<InputActionHolder *> (++holderCursor);
            holder->action = _control->onChangedAction;
            holder->action.real[0u] = _control->floatValue;
            holder->dispatchType = _control->onChangedActionDispatch;
        }

        break;
    }
}

void UIProcessor::ProcessControl (const LabelControl *_control) noexcept
{
    ImGui::TextUnformatted (_control->label.c_str ());
}

void UIProcessor::ProcessControl (WindowControl *_control) noexcept
{
    if (!_control->open)
    {
        return;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
    if (!_control->minimizable)
    {
        flags |= ImGuiWindowFlags_NoCollapse;
    }

    if (!_control->minimizable)
    {
        flags |= ImGuiWindowFlags_NoCollapse;
    }

    if (!_control->resizable)
    {
        flags |= ImGuiWindowFlags_NoResize;
    }

    if (!_control->movable)
    {
        flags |= ImGuiWindowFlags_NoMove;
    }

    if (!_control->hasTitleBar)
    {
        flags |= ImGuiWindowFlags_NoTitleBar;
    }

    if (_control->pack)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;
    }

    const auto anchorX = static_cast<int32_t> (ImGui::GetIO ().DisplaySize.x * _control->anchor.x);
    const auto anchorY = static_cast<int32_t> (ImGui::GetIO ().DisplaySize.y * _control->anchor.y);

    ImGui::SetNextWindowPos ({static_cast<float> (anchorX + _control->x), static_cast<float> (anchorY + _control->y)});
    ImGui::SetNextWindowSize ({static_cast<float> (_control->width), static_cast<float> (_control->height)});

    if (ImGui::Begin (_control->title.c_str (), _control->closable ? &_control->open : nullptr, flags))
    {
        NodeOrderingStack::Sequence orderingSequence {&nodeOrderingStack};
        for (auto nodeCursor = fetchNodeByParentId.Execute (&_control->nodeId);
             const auto *node = static_cast<const UINode *> (*nodeCursor); ++nodeCursor)
        {
            orderingSequence.Add (node->nodeId, node->sortIndex);
        }

        orderingSequence.Sort ();
        ProcessChildControls (orderingSequence, _control->layout);
    }

    if (!_control->open)
    {
        auto holderCursor = insertInputActionHolder.Execute ();
        auto *holder = static_cast<InputActionHolder *> (++holderCursor);
        holder->action = _control->onClosedAction;
        holder->dispatchType = _control->onClosedActionDispatch;
    }

    _control->x = static_cast<int32_t> (ImGui::GetWindowPos ().x) - anchorX;
    _control->y = static_cast<int32_t> (ImGui::GetWindowPos ().y) - anchorY;

    _control->width = static_cast<uint32_t> (ImGui::GetWindowWidth ());
    _control->height = static_cast<uint32_t> (ImGui::GetWindowHeight ());
    ImGui::End ();
}

void UIProcessor::ProcessChildControls (const NodeOrderingStack::Sequence &_sequence,
                                        ContainerControlLayout _layout) noexcept
{
    bool first = true;
    for (const NodeInfo &info : _sequence)
    {
        switch (_layout)
        {
        case ContainerControlLayout::VERTICAL:
            break;

        case ContainerControlLayout::HORIZONTAL:
            if (!first)
            {
                ImGui::SameLine ();
            }

            first = false;
            break;
        }

        auto nodeCursor = fetchNodeByNodeId.Execute (&info.nodeId);
        const auto *node = static_cast<const UINode *> (*nodeCursor);
        ProcessNode (node);
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        FrameInputAccumulator *_inputAccumulator,
                        const KeyCodeMapping &_keyCodeMapping) noexcept
{
    _pipelineBuilder.AddTask ("UIProcessor"_us).SetExecutor<UIProcessor> (_inputAccumulator, _keyCodeMapping);
}
} // namespace Emergence::Celerity::UIProcessing
