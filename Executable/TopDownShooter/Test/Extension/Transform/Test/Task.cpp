#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <Transform/Test/Task.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dVisualSync.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Transform::RequestExecutor
{
class Executor final : public Celerity::TaskExecutorBase<Executor>
{
public:
    Executor (Celerity::TaskConstructor &_constructor, Container::Vector<RequestPacket> _requests) noexcept;

    void Execute () noexcept;

private:
    Container::Vector<RequestPacket> requests;
    std::size_t executionIndex = 0u;

    Celerity::InsertLongTermQuery insertTransform;
    Celerity::FetchValueQuery fetchTransform;
    Celerity::ModifyValueQuery modifyTransform;
    Transform3dWorldAccessor worldAccessor;
};

Executor::Executor (Celerity::TaskConstructor &_constructor, Container::Vector<RequestPacket> _requests) noexcept
    : requests (std::move (_requests)),
      insertTransform (_constructor.InsertLongTerm (Transform3dComponent::Reflect ().mapping)),
      fetchTransform (_constructor.FetchValue (Transform3dComponent::Reflect ().mapping,
                                               {Transform3dComponent::Reflect ().objectId})),
      modifyTransform (_constructor.ModifyValue (Transform3dComponent::Reflect ().mapping,
                                                 {Transform3dComponent::Reflect ().objectId})),
      worldAccessor (_constructor)
{
}

void Executor::Execute () noexcept
{
    REQUIRE (executionIndex < requests.size ());

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

                            CHECK (NearlyEqual (transform, _request.expectedTransform));
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
            },
            request);
    }

    ++executionIndex;
}

void AddToFixedUpdate (Celerity::PipelineBuilder &_pipelineBuilder, Container::Vector<RequestPacket> _requests) noexcept
{
    Emergence::Celerity::TaskConstructor constructor =
        _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});
    constructor.SetExecutor<Executor> (std::move (_requests));
}

void AddToNormalUpdate (Celerity::PipelineBuilder &_pipelineBuilder,
                        Container::Vector<RequestPacket> _requests) noexcept
{
    Emergence::Celerity::TaskConstructor constructor =
        _pipelineBuilder.AddTask (Memory::UniqueString {"TransformRequestExecutor"});

    constructor.DependOn (VisualSync::Checkpoint::SYNC_FINISHED);
    constructor.SetExecutor<Executor> (std::move (_requests));
}
} // namespace Emergence::Transform::RequestExecutor
