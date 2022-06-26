#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
/// \brief Encapsulates query, required for Transform3dComponent world transform access.
class Transform3dWorldAccessor final
{
public:
    Transform3dWorldAccessor (TaskConstructor &_constructor) noexcept;

private:
    friend class Transform3dComponent;

    FetchValueQuery fetchTransform3dByObjectId;
};
} // namespace Emergence::Celerity
