#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dHierarchyCleanup.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>
#include <Celerity/Transform/Transform3dWorldAccessor.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test::RequestExecutor
{
class Executor final : public TaskExecutorBase<Executor>
{
public:
    Executor (TaskConstructor &_constructor, Container::Vector<RequestPacket> _requests) noexcept;

    void Execute () noexcept;

private:
    Container::Vector<RequestPacket> requests;
    std::size_t executionIndex = 0u;

    InsertLongTermQuery insertTransform;
    FetchValueQuery fetchTransform;
    ModifyValueQuery modifyTransform;
    Transform3dWorldAccessor worldAccessor;
};

Executor::Executor (TaskConstructor &_constructor, Container::Vector<RequestPacket> _requests) noexcept
    : requests (std::move (_requests)),
      insertTransform (INSERT_LONG_TERM (Transform3dComponent)),
      fetchTransform (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      modifyTransform (MODIFY_VALUE_1F (Transform3dComponent, objectId)),
      worldAccessor (_constructor)
{
}

void Executor::Execute () noexcept
{
    if (executionIndex >= requests.size ())
    {
        return;
    }

#define TRANSFORM_LOG_SEQUENCE(transform)                                                                              \
    "translation {", (transform).translation.x, ", ", (transform).translation.y, ", ", (transform).translation.z,      \
        "} rotation {", (transform).rotation.x, ", ", (transform).rotation.y, ", ", (transform).rotation.z, ", ",      \
        (transform).rotation.w, "} scale {", (transform).scale.x, ", ", (transform).scale.y, ", ",                     \
        (transform).scale.z, "}"

    for (const Request &request : requests[executionIndex])
    {
        std::visit (
            [this] (const auto &_request)
            {
                using Type = std::decay_t<decltype (_request)>;
                if constexpr (std::is_same_v<Type, Requests::CreateTransform>)
                {
                    LOG ("Creating transform with id ", _request.id, " and parent id ", _request.parentId, ".");

                    auto cursor = insertTransform.Execute ();
                    auto *component = static_cast<Transform3dComponent *> (++cursor);
                    component->SetObjectId (_request.id);
                    component->SetParentObjectId (_request.parentId);
                }
                else if constexpr (std::is_same_v<Type, Requests::ChangeParent>)
                {
                    LOG ("Changing transform with id ", _request.id, " parent id to ", _request.newParentId, ".");
                    auto cursor = modifyTransform.Execute (&_request.id);

                    if (auto *component = static_cast<Transform3dComponent *> (*cursor))
                    {
                        component->SetParentObjectId (_request.newParentId);
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find transform with required id!");
                    }
                }
                else if constexpr (std::is_same_v<Type, Requests::SetLocalTransform>)
                {
                    LOG ("Setting local ", _request.logical ? "logical" : "visual", " transform with id ", _request.id,
                         _request.skipInterpolation ? " (skipping logical transform interpolation)" : "",
                         " to value: ", TRANSFORM_LOG_SEQUENCE (_request.transform));

                    auto cursor = modifyTransform.Execute (&_request.id);
                    if (auto *component = static_cast<Transform3dComponent *> (*cursor))
                    {
                        if (_request.logical)
                        {
                            component->SetLogicalLocalTransform (_request.transform, _request.skipInterpolation);
                        }
                        else
                        {
                            component->SetVisualLocalTransform (_request.transform);
                        }
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find transform with required id!");
                    }
                }
                else if constexpr (std::is_same_v<Type, Requests::CheckTransform>)
                {
                    LOG ("Checking that ", _request.local ? "local " : "world ",
                         _request.logical ? "logical" : "visual", " transform with id ", _request.id,
                         " equals to value: ", TRANSFORM_LOG_SEQUENCE (_request.expectedTransform),
                         _request.useModifyQuery ? " (using modify query)" : "", ".");

                    auto executeWithQuery = [this, &_request] (auto &_query)
                    {
                        auto cursor = _query.Execute (&_request.id);
                        if (const auto *component = static_cast<const Transform3dComponent *> (*cursor))
                        {
                            Math::Transform3d transform;
                            if (_request.local)
                            {
                                transform = _request.logical ? component->GetLogicalLocalTransform () :
                                                               component->GetVisualLocalTransform ();
                            }
                            else
                            {
                                transform = _request.logical ? component->GetLogicalWorldTransform (worldAccessor) :
                                                               component->GetVisualWorldTransform (worldAccessor);
                            }

                            const bool equal = NearlyEqual (transform, _request.expectedTransform);
                            CHECK_WITH_MESSAGE (
                                equal, "Real transform: ", TRANSFORM_LOG_SEQUENCE (transform),
                                ". Expected transform: ", TRANSFORM_LOG_SEQUENCE (_request.expectedTransform), ".");
                        }
                        else
                        {
                            CHECK_WITH_MESSAGE (false, "Unable to find transform with required id!");
                        }
                    };

                    if (_request.useModifyQuery)
                    {
                        auto unsafeAccess = modifyTransform.AllowUnsafeFetchAccess ();
                        executeWithQuery (modifyTransform);
                    }
                    else
                    {
                        executeWithQuery (fetchTransform);
                    }
                }
                else if constexpr (std::is_same_v<Type, Requests::CheckTransformExists>)
                {
                    LOG ("Checking that transform with id ", _request.id, _request.exists ? " " : " does not ",
                         "exists.");
                    auto cursor = modifyTransform.Execute (&_request.id);

                    if (_request.exists)
                    {
                        CHECK_NOT_EQUAL (*cursor, nullptr);
                    }
                    else
                    {
                        CHECK_EQUAL (*cursor, nullptr);
                    }
                }
                else if constexpr (std::is_same_v<Type, Requests::RemoveTransform>)
                {
                    LOG ("Removing transform with id ", _request.id, ".");
                    auto cursor = modifyTransform.Execute (&_request.id);

                    if (*cursor)
                    {
                        ~cursor;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find transform with required id!");
                    }
                }
            },
            request);
    }

    ++executionIndex;
}

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder,
                       Container::Vector<RequestPacket> _requests,
                       bool _withHierarchyCleanup) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    if (_withHierarchyCleanup)
    {
        constructor.DependOn (HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);
        constructor.MakeDependencyOf (HierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_STARTED);
    }

    constructor.SetExecutor<Executor> (std::move (_requests));
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.DependOn (VisualTransformSync::Checkpoint::FINISHED);
    constructor.SetExecutor<Executor> (std::move (_requests));
}
} // namespace Emergence::Celerity::Test::RequestExecutor
