#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace Emergence::Transform
{
Transform3dWorldAccessor::Transform3dWorldAccessor (Celerity::TaskConstructor &_constructor) noexcept
    : fetchTransform3dByObjectId (_constructor.FetchValue (Transform3dComponent::Reflect ().mapping,
                                                           {Transform3dComponent::Reflect ().objectId}))
{
}
} // namespace Emergence::Transform
