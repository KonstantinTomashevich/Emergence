#pragma once

#include <CelerityTransformApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

#include <Math/Transform2d.hpp>
#include <Math/Transform3d.hpp>

namespace Emergence::Celerity
{
/// \brief Encapsulates query, required for Transform3dComponent world transform access.
template <typename Transform>
class CelerityTransformApi TransformWorldAccessor final
{
public:
    TransformWorldAccessor (TaskConstructor &_constructor) noexcept;

private:
    friend class TransformComponent<Transform>;

    FetchValueQuery fetchTransformByObjectId;
};

using Transform2dWorldAccessor = TransformWorldAccessor<Math::Transform2d>;
using Transform3dWorldAccessor = TransformWorldAccessor<Math::Transform3d>;
} // namespace Emergence::Celerity
