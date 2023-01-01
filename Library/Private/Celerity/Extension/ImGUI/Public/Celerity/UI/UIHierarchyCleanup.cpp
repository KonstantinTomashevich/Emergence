#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/UI/ButtonControl.hpp>
#include <Celerity/UI/CheckboxControl.hpp>
#include <Celerity/UI/ContainerControl.hpp>
#include <Celerity/UI/Events.hpp>
#include <Celerity/UI/ImageControl.hpp>
#include <Celerity/UI/InputControl.hpp>
#include <Celerity/UI/LabelControl.hpp>
#include <Celerity/UI/UIHierarchyCleanup.hpp>
#include <Celerity/UI/UINode.hpp>
#include <Celerity/UI/UIProcessing.hpp>
#include <Celerity/UI/WindowControl.hpp>

namespace Emergence::Celerity::UIHierarchyCleanup
{
namespace TaskNames
{
static const Memory::UniqueString DETECTOR {"UIHierarchyCleanup::Detector"};
static const Memory::UniqueString REMOVER {"UIHierarchyCleanup::Remover"};
} // namespace TaskNames

struct UICleanupMessage final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const UICleanupMessage::Reflection &UICleanupMessage::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UICleanupMessage);
        EMERGENCE_MAPPING_REGISTER_REGULAR (nodeId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

class Detector final : public TaskExecutorBase<Detector>
{
public:
    Detector (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void OnNodeRemoved (UniqueId _nodeId) noexcept;

    FetchSequenceQuery fetchNodeRemovedEvents;
    FetchValueQuery fetchNodeByParentId;
    InsertShortTermQuery insertCleanupMessage;
};

Detector::Detector (TaskConstructor &_constructor) noexcept
    : fetchNodeRemovedEvents (FETCH_SEQUENCE (UINodeRemovedNormalEvent)),
      fetchNodeByParentId (FETCH_VALUE_1F (UINode, parentId)),
      insertCleanupMessage (INSERT_SHORT_TERM (UICleanupMessage))
{
}

void Detector::Execute () noexcept
{
    for (auto eventCursor = fetchNodeRemovedEvents.Execute ();
         const auto *event = static_cast<const UINodeRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        OnNodeRemoved (event->nodeId);
    }
}

void Detector::OnNodeRemoved (UniqueId _nodeId) noexcept
{
    {
        auto messageCursor = insertCleanupMessage.Execute ();
        static_cast<UICleanupMessage *> (++messageCursor)->nodeId = _nodeId;
    }

    for (auto childCursor = fetchNodeByParentId.Execute (&_nodeId);
         const auto *node = static_cast<const UINode *> (*childCursor); ++childCursor)
    {
        OnNodeRemoved (node->nodeId);
    }
}

class Remover final : public TaskExecutorBase<Remover>
{
public:
    Remover (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    ModifySequenceQuery modifyCleanupMessages;
    RemoveValueQuery removeButtonByNodeId;
    RemoveValueQuery removeCheckboxByNodeId;
    RemoveValueQuery removeContainerByNodeId;
    RemoveValueQuery removeImageByNodeId;
    RemoveValueQuery removeInputByNodeId;
    RemoveValueQuery removeLabelByNodeId;
    RemoveValueQuery removeNodeByNodeId;
    RemoveValueQuery removeWindowByNodeId;
};

Remover::Remover (TaskConstructor &_constructor) noexcept
    : modifyCleanupMessages (MODIFY_SEQUENCE (UICleanupMessage)),
      removeButtonByNodeId (REMOVE_VALUE_1F (ButtonControl, nodeId)),
      removeCheckboxByNodeId (REMOVE_VALUE_1F (CheckboxControl, nodeId)),
      removeContainerByNodeId (REMOVE_VALUE_1F (ContainerControl, nodeId)),
      removeImageByNodeId (REMOVE_VALUE_1F (ImageControl, nodeId)),
      removeInputByNodeId (REMOVE_VALUE_1F (InputControl, nodeId)),
      removeLabelByNodeId (REMOVE_VALUE_1F (LabelControl, nodeId)),
      removeNodeByNodeId (REMOVE_VALUE_1F (UINode, nodeId)),
      removeWindowByNodeId (REMOVE_VALUE_1F (WindowControl, nodeId))
{
    _constructor.DependOn (TaskNames::DETECTOR);
    _constructor.MakeDependencyOf (UIProcessing::Checkpoint::STARTED);
}

void Remover::Execute () noexcept
{
    for (auto messageCursor = modifyCleanupMessages.Execute ();
         const auto *message = static_cast<const UICleanupMessage *> (*messageCursor); ~messageCursor)
    {
        if (auto nodeCursor = removeNodeByNodeId.Execute (&message->nodeId); nodeCursor.ReadConst ())
        {
            ~nodeCursor;
        }

        if (auto buttonCursor = removeButtonByNodeId.Execute (&message->nodeId); buttonCursor.ReadConst ())
        {
            ~buttonCursor;
        }
        else if (auto checkboxCursor = removeCheckboxByNodeId.Execute (&message->nodeId); checkboxCursor.ReadConst ())
        {
            ~checkboxCursor;
        }
        else if (auto containerCursor = removeContainerByNodeId.Execute (&message->nodeId);
                 containerCursor.ReadConst ())
        {
            ~containerCursor;
        }
        else if (auto imageCursor = removeImageByNodeId.Execute (&message->nodeId); imageCursor.ReadConst ())
        {
            ~imageCursor;
        }
        else if (auto inputCursor = removeInputByNodeId.Execute (&message->nodeId); inputCursor.ReadConst ())
        {
            ~inputCursor;
        }
        else if (auto labelCursor = removeLabelByNodeId.Execute (&message->nodeId); labelCursor.ReadConst ())
        {
            ~labelCursor;
        }
        else if (auto windowCursor = removeWindowByNodeId.Execute (&message->nodeId); windowCursor.ReadConst ())
        {
            ~windowCursor;
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::DETECTOR).SetExecutor<Detector> ();
    _pipelineBuilder.AddTask (TaskNames::REMOVER).SetExecutor<Remover> ();
}
} // namespace Emergence::Celerity::UIHierarchyCleanup
