#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_PADDING_WARNING

#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
namespace Requests
{
SetLocalTransform::SetLocalTransform (Celerity::UniqueId _id,
                                      bool _logical,
                                      bool _skipInterpolation,
                                      const Math::Transform2d &_transform2d) noexcept
    : id (_id),
      logical (_logical),
      skipInterpolation (_skipInterpolation),
      transform2d (_transform2d)
{
}

SetLocalTransform::SetLocalTransform (Celerity::UniqueId _id,
                                      bool _logical,
                                      bool _skipInterpolation,
                                      const Math::Transform3d &_transform3d) noexcept
    : id (_id),
      logical (_logical),
      skipInterpolation (_skipInterpolation),
      transform3d (_transform3d)
{
}

CheckTransform::CheckTransform (UniqueId _id,
                                bool _logical,
                                bool _local,
                                bool _useModifyQuery,
                                const Math::Transform2d &_expectedTransform2D) noexcept
    : id (_id),
      logical (_logical),
      local (_local),
      useModifyQuery (_useModifyQuery),
      expectedTransform2d (_expectedTransform2D)
{
}

CheckTransform::CheckTransform (UniqueId _id,
                                bool _logical,
                                bool _local,
                                bool _useModifyQuery,
                                const Math::Transform3d &_expectedTransform3D) noexcept
    : id (_id),
      logical (_logical),
      local (_local),
      useModifyQuery (_useModifyQuery),
      expectedTransform3d (_expectedTransform3D)
{
}
} // namespace Requests

namespace RequestExecutor
{
template <typename Transform>
class Executor final : public TaskExecutorBase<Executor<Transform>>
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
    TransformWorldAccessor<Transform> worldAccessor;
};

template <typename Transform>
Executor<Transform>::Executor (TaskConstructor &_constructor, Container::Vector<RequestPacket> _requests) noexcept
    : requests (std::move (_requests)),
      insertTransform (INSERT_LONG_TERM (TransformComponent<Transform>)),
      fetchTransform (FETCH_VALUE_1F (TransformComponent<Transform>, objectId)),
      modifyTransform (MODIFY_VALUE_1F (TransformComponent<Transform>, objectId)),
      worldAccessor (_constructor)
{
}

template <typename Transform>
void Executor<Transform>::Execute () noexcept
{
    if (executionIndex >= requests.size ())
    {
        return;
    }

    auto transformToString = [] (const Transform &_transform) -> Container::String
    {
        if constexpr (std::is_same_v<Transform, Math::Transform2d>)
        {
            return EMERGENCE_BUILD_STRING ("translation {", _transform.translation.x, ", ", _transform.translation.y,
                                           "} rotation {", _transform.rotation, "} scale {", _transform.scale.x, ", ",
                                           _transform.scale.y, "}");
        }
        else if constexpr (std::is_same_v<Transform, Math::Transform3d>)
        {
            return EMERGENCE_BUILD_STRING ("translation {", _transform.translation.x, ", ", _transform.translation.y,
                                           ", ", _transform.translation.z, "} rotation {", _transform.rotation.x, ", ",
                                           _transform.rotation.y, ", ", _transform.rotation.z, ", ",
                                           _transform.rotation.w, "} scale {", _transform.scale.x, ", ",
                                           _transform.scale.y, ", ", _transform.scale.z, "}");
        }
        else
        {
            return "Unknown transform";
        }
    };

    for (const Request &request : requests[executionIndex])
    {
        std::visit (
            [this, &transformToString] (const auto &_request)
            {
                using Type = std::decay_t<decltype (_request)>;
                if constexpr (std::is_same_v<Type, Requests::CreateTransform>)
                {
                    LOG ("Creating transform with id ", _request.id, " and parent id ", _request.parentId, ".");

                    auto cursor = insertTransform.Execute ();
                    auto *component = static_cast<TransformComponent<Transform> *> (++cursor);
                    component->SetObjectId (_request.id);
                    component->SetParentObjectId (_request.parentId);
                }
                else if constexpr (std::is_same_v<Type, Requests::ChangeParent>)
                {
                    LOG ("Changing transform with id ", _request.id, " parent id to ", _request.newParentId, ".");
                    auto cursor = modifyTransform.Execute (&_request.id);

                    if (auto *component = static_cast<TransformComponent<Transform> *> (*cursor))
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
                    Transform transform;
                    if constexpr (std::is_same_v<Transform, Math::Transform2d>)
                    {
                        transform = _request.transform2d;
                    }
                    else if constexpr (std::is_same_v<Transform, Math::Transform3d>)
                    {
                        transform = _request.transform3d;
                    }

                    LOG ("Setting local ", _request.logical ? "logical" : "visual", " transform with id ", _request.id,
                         _request.skipInterpolation ? " (skipping logical transform interpolation)" : "",
                         " to value: ", transformToString (transform));

                    auto cursor = modifyTransform.Execute (&_request.id);
                    if (auto *component = static_cast<TransformComponent<Transform> *> (*cursor))
                    {
                        if (_request.logical)
                        {
                            component->SetLogicalLocalTransform (transform, _request.skipInterpolation);
                        }
                        else
                        {
                            component->SetVisualLocalTransform (transform);
                        }
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find transform with required id!");
                    }
                }
                else if constexpr (std::is_same_v<Type, Requests::CheckTransform>)
                {
                    Transform expectedTransform;
                    if constexpr (std::is_same_v<Transform, Math::Transform2d>)
                    {
                        expectedTransform = _request.expectedTransform2d;
                    }
                    else if constexpr (std::is_same_v<Transform, Math::Transform3d>)
                    {
                        expectedTransform = _request.expectedTransform3d;
                    }

                    LOG ("Checking that ", _request.local ? "local " : "world ",
                         _request.logical ? "logical" : "visual", " transform with id ", _request.id,
                         " equals to value: ", transformToString (expectedTransform),
                         _request.useModifyQuery ? " (using modify query)" : "", ".");

                    auto executeWithQuery = [this, &_request, &expectedTransform, &transformToString] (auto &_query)
                    {
                        auto cursor = _query.Execute (&_request.id);
                        if (const auto *component = static_cast<const TransformComponent<Transform> *> (*cursor))
                        {
                            Transform transform;
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

                            const bool equal = NearlyEqual (transform, expectedTransform);
                            CHECK_WITH_MESSAGE (equal, "Real transform: ", transformToString (transform),
                                                ". Expected transform: ", transformToString (expectedTransform), ".");
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

void Add2dToFixedUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    constructor.SetExecutor<Executor<Math::Transform2d>> (std::move (_requests));
}

void Add2dToNormalUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    constructor.DependOn (TransformVisualSync::Checkpoint::FINISHED);
    constructor.SetExecutor<Executor<Math::Transform2d>> (std::move (_requests));
}

void Add3dToFixedUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    constructor.SetExecutor<Executor<Math::Transform3d>> (std::move (_requests));
}

void Add3dToNormalUpdate (PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    TaskConstructor constructor = _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    constructor.DependOn (TransformVisualSync::Checkpoint::FINISHED);
    constructor.SetExecutor<Executor<Math::Transform3d>> (std::move (_requests));
}
} // namespace RequestExecutor
} // namespace Emergence::Celerity::Test

END_MUTING_WARNINGS
