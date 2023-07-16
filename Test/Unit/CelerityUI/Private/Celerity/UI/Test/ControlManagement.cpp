#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/UI/Test/ControlManagement.hpp>
#include <Celerity/UI/UI.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIRenderPass.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test::ControlManagement
{
class ControlManager final : public TaskExecutorBase<ControlManager>
{
public:
    ControlManager (TaskConstructor &_constructor, Container::Vector<Frame> _frames) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertViewport;
    InsertLongTermQuery insertPass;

    InsertLongTermQuery insertNode;
    InsertLongTermQuery insertButton;
    InsertLongTermQuery insertCheckbox;
    InsertLongTermQuery insertContainer;
    InsertLongTermQuery insertImage;
    InsertLongTermQuery insertInput;
    InsertLongTermQuery insertLabel;
    InsertLongTermQuery insertWindow;

    InsertLongTermQuery insertColorProperty;
    InsertLongTermQuery insertFloatProperty;
    InsertLongTermQuery insertFloatPairProperty;
    InsertLongTermQuery insertFontProperty;

    RemoveValueQuery removeNodeById;

    std::uint64_t currentFrameIndex = 0u;
    Container::Vector<Frame> frames;
};

ControlManager::ControlManager (TaskConstructor &_constructor, Container::Vector<Frame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      insertViewport (INSERT_LONG_TERM (Viewport)),
      insertPass (INSERT_LONG_TERM (UIRenderPass)),

      insertNode (INSERT_LONG_TERM (UINode)),
      insertButton (INSERT_LONG_TERM (ButtonControl)),
      insertCheckbox (INSERT_LONG_TERM (CheckboxControl)),
      insertContainer (INSERT_LONG_TERM (ContainerControl)),
      insertImage (INSERT_LONG_TERM (ImageControl)),
      insertInput (INSERT_LONG_TERM (InputControl)),
      insertLabel (INSERT_LONG_TERM (LabelControl)),
      insertWindow (INSERT_LONG_TERM (WindowControl)),

      insertColorProperty (INSERT_LONG_TERM (UIStyleColorProperty)),
      insertFloatProperty (INSERT_LONG_TERM (UIStyleFloatProperty)),
      insertFloatPairProperty (INSERT_LONG_TERM (UIStyleFloatPairProperty)),
      insertFontProperty (INSERT_LONG_TERM (UIStyleFontProperty)),

      removeNodeById (REMOVE_VALUE_1F (UINode, nodeId)),

      frames (std::move (_frames))
{
    _constructor.DependOn (UI::Checkpoint::HIERARCHY_CLEANUP_FINISHED);
    _constructor.MakeDependencyOf (UI::Checkpoint::UPDATE_STARTED);
}

void ControlManager::Execute () noexcept
{
    if (currentFrameIndex >= frames.size ())
    {
        return;
    }

    auto createNode = [this] (UniqueId _nodeId, UniqueId _parentId, Memory::UniqueString _styleId)
    {
        auto nodeCursor = insertNode.Execute ();
        auto *node = static_cast<UINode *> (++nodeCursor);
        node->nodeId = _nodeId;
        node->parentId = _parentId;
        node->styleId = _styleId;
        node->sortIndex = _nodeId;
    };

    for (const Task &task : frames[currentFrameIndex])
    {
        std::visit (
            [this, &createNode] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, Tasks::CreateViewport>)
                {
                    LOG ("Creating viewport with name \"", _task.viewportName, "\"...");
                    auto viewportCursor = insertViewport.Execute ();
                    auto *viewport = static_cast<Viewport *> (++viewportCursor);

                    viewport->name = _task.viewportName;
                    viewport->sortIndex = _task.sortIndex;
                    viewport->x = _task.x;
                    viewport->y = _task.y;
                    viewport->width = _task.width;
                    viewport->height = _task.height;
                    viewport->clearColor = _task.clearColor;
                    viewport->sortMode = Render::Backend::ViewportSortMode::SEQUENTIAL;

                    auto passCursor = insertPass.Execute ();
                    auto *pass = static_cast<UIRenderPass *> (++passCursor);
                    pass->name = _task.viewportName;
                    pass->defaultStyleId = _task.defaultStyleId;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateButton>)
                {
                    LOG ("Creating button with id ", _task.nodeId, " and label \"", _task.label, "\"...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto buttonCursor = insertButton.Execute ();
                    auto *button = static_cast<ButtonControl *> (++buttonCursor);

                    button->nodeId = _task.nodeId;
                    button->labelKey = _task.labelKey;
                    button->label = _task.label;
                    button->width = _task.width;
                    button->height = _task.height;
                    button->onClickAction = _task.onClickAction;
                    button->onClickActionDispatch = _task.onClickActionDispatch;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateCheckbox>)
                {
                    LOG ("Creating checkbox with id ", _task.nodeId, " and label \"", _task.label, "\"...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto checkboxCursor = insertCheckbox.Execute ();
                    auto *checkbox = static_cast<CheckboxControl *> (++checkboxCursor);

                    checkbox->nodeId = _task.nodeId;
                    checkbox->labelKey = _task.labelKey;
                    checkbox->label = _task.label;
                    checkbox->checked = _task.checked;
                    checkbox->onChangedAction = _task.onChangedAction;
                    checkbox->onChangedActionDispatch = _task.onChangedActionDispatch;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateContainer>)
                {
                    LOG ("Creating container with id ", _task.nodeId, "...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto containerCursor = insertContainer.Execute ();
                    auto *container = static_cast<ContainerControl *> (++containerCursor);

                    container->nodeId = _task.nodeId;
                    container->type = _task.type;
                    container->layout = _task.layout;
                    container->width = _task.width;
                    container->height = _task.height;
                    container->border = _task.border;
                    container->labelKey = _task.labelKey;
                    container->label = _task.label;
                    container->previewKey = _task.previewKey;
                    container->preview = _task.preview;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateImage>)
                {
                    LOG ("Creating image with id ", _task.nodeId, " and texture \"", _task.textureId, "\"...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto imageCursor = insertImage.Execute ();
                    auto *image = static_cast<ImageControl *> (++imageCursor);

                    image->nodeId = _task.nodeId;
                    image->textureId = _task.textureId;
                    image->width = _task.width;
                    image->height = _task.height;
                    image->uv = _task.uv;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateInput>)
                {
                    LOG ("Creating input with id ", _task.nodeId, " and label \"", _task.label, "\"...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto inputCursor = insertInput.Execute ();
                    auto *input = static_cast<InputControl *> (++inputCursor);

                    input->nodeId = _task.nodeId;
                    input->type = _task.type;
                    input->labelKey = _task.labelKey;
                    input->label = _task.label;
                    input->onChangedAction = _task.onChangedAction;
                    input->onChangedActionDispatch = _task.onChangedActionDispatch;

                    switch (input->type)
                    {
                    case InputControlType::TEXT:
                        input->utf8TextValue = _task.utf8TextValue;
                        break;

                    case InputControlType::INT:
                        input->intValue = _task.intValue;
                        break;

                    case InputControlType::FLOAT:
                        input->floatValue = _task.floatValue;
                        break;
                    }
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateLabel>)
                {
                    LOG ("Creating label with id ", _task.nodeId, " and text \"", _task.label, "\"...");
                    createNode (_task.nodeId, _task.parentId, _task.styleId);
                    auto labelCursor = insertLabel.Execute ();
                    auto *label = static_cast<LabelControl *> (++labelCursor);

                    label->nodeId = _task.nodeId;
                    label->labelKey = _task.labelKey;
                    label->label = _task.label;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateWindow>)
                {
                    LOG ("Creating window with id ", _task.nodeId, " and title \"", _task.title, "\"...");
                    createNode (_task.nodeId, INVALID_UNIQUE_ID, _task.styleId);
                    auto windowCursor = insertWindow.Execute ();
                    auto *window = static_cast<WindowControl *> (++windowCursor);

                    window->nodeId = _task.nodeId;
                    window->viewportName = _task.viewportName;
                    window->titleKey = _task.titleKey;
                    window->title = _task.title;

                    window->closable = _task.closable;
                    window->minimizable = _task.minimizable;
                    window->resizable = _task.resizable;
                    window->movable = _task.movable;
                    window->hasTitleBar = _task.hasTitleBar;
                    window->pack = _task.pack;

                    window->open = _task.open;
                    window->layout = _task.layout;

                    window->anchor = _task.anchor;
                    window->pivot = _task.pivot;

                    window->x = _task.x;
                    window->y = _task.y;
                    window->width = _task.width;
                    window->height = _task.height;

                    window->onClosedAction = _task.onClosedAction;
                    window->onClosedActionDispatch = _task.onClosedActionDispatch;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateStyleColorProperty>)
                {
                    LOG ("Creating color property for style \"", _task.styleId, "\"...");
                    auto propertyCursor = insertColorProperty.Execute ();
                    auto *property = static_cast<UIStyleColorProperty *> (++propertyCursor);
                    property->styleId = _task.styleId;
                    property->property = _task.property;
                    property->red = _task.red;
                    property->green = _task.green;
                    property->blue = _task.blue;
                    property->alpha = _task.alpha;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateStyleFloatProperty>)
                {
                    LOG ("Creating float property for style \"", _task.styleId, "\"...");
                    auto propertyCursor = insertFloatProperty.Execute ();
                    auto *property = static_cast<UIStyleFloatProperty *> (++propertyCursor);
                    property->styleId = _task.styleId;
                    property->property = _task.property;
                    property->value = _task.value;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateStyleFloatPairProperty>)
                {
                    LOG ("Creating float pair property for style \"", _task.styleId, "\"...");
                    auto propertyCursor = insertFloatPairProperty.Execute ();
                    auto *property = static_cast<UIStyleFloatPairProperty *> (++propertyCursor);
                    property->styleId = _task.styleId;
                    property->property = _task.property;
                    property->x = _task.x;
                    property->y = _task.y;
                }
                else if constexpr (std::is_same_v<Type, Tasks::CreateStyleFontProperty>)
                {
                    LOG ("Creating font property for style \"", _task.styleId, "\"...");
                    auto propertyCursor = insertFontProperty.Execute ();
                    auto *property = static_cast<UIStyleFontProperty *> (++propertyCursor);
                    property->styleId = _task.styleId;
                    property->fontId = _task.fontId;
                }
                else if constexpr (std::is_same_v<Type, Tasks::RemoveControl>)
                {
                    LOG ("Removing node with id ", _task.nodeId, "...");
                    auto removeCursor = removeNodeById.Execute (&_task.nodeId);

                    if (removeCursor.ReadConst ())
                    {
                        ~removeCursor;
                    }
                }
            },
            task);
    }

    ++currentFrameIndex;
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<Container::Vector<Task>> _frames) noexcept
{
    _pipelineBuilder.AddTask (Memory::UniqueString {"ControlManager"})
        .SetExecutor<ControlManager> (std::move (_frames));
}
} // namespace Emergence::Celerity::Test::ControlManagement
