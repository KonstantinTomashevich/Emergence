#include <Celerity/PipelineBuilderMacros.hpp>

#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Transform
{
Transform3dWorldAccessor::Transform3dWorldAccessor (Celerity::TaskConstructor &_constructor) noexcept
    : fetchTransform3dByObjectId (_constructor.MFetchValue1F (Transform3dComponent, objectId))
{
}
} // namespace Emergence::Transform
