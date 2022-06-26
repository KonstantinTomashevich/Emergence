#include <Celerity/PipelineBuilderMacros.hpp>

#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Celerity
{
Transform3dWorldAccessor::Transform3dWorldAccessor (TaskConstructor &_constructor) noexcept
    : fetchTransform3dByObjectId (FETCH_VALUE_1F (Transform3dComponent, objectId))
{
}
} // namespace Emergence::Celerity
