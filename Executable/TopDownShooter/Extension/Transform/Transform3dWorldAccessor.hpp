#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Transform
{
/// \brief Holds query, required for Transform3dComponent world transform access.
class Transform3dWorldAccessor final
{
public:
    Transform3dWorldAccessor (Celerity::TaskConstructor &_constructor) noexcept;

private:
    friend class Transform3dComponent;

    Celerity::FetchValueQuery fetchTransform3dByObjectId;
};
} // namespace Emergence::Transform
