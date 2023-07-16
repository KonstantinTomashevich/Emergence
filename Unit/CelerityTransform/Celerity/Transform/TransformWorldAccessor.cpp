#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

namespace Emergence::Celerity
{
template <typename Transform>
TransformWorldAccessor<Transform>::TransformWorldAccessor (TaskConstructor &_constructor) noexcept
    : fetchTransformByObjectId (FETCH_VALUE_1F (TransformComponent<Transform>, objectId))
{
}

template class TransformWorldAccessor<Math::Transform2d>;
template class TransformWorldAccessor<Math::Transform3d>;
} // namespace Emergence::Celerity
