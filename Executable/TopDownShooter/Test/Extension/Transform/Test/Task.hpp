#pragma once

#include <variant>

#include <Celerity/Constants.hpp>
#include <Celerity/PipelineBuilder.hpp>

#include <Container/Vector.hpp>

#include <Math/Transform3d.hpp>

namespace Emergence::Transform::Test
{
namespace Requests
{
struct CreateTransform final
{
    Celerity::ObjectId id = Celerity::INVALID_OBJECT_ID;
    Celerity::ObjectId parentId = Celerity::INVALID_OBJECT_ID;
};

struct ChangeParent final
{
    Celerity::ObjectId id = Celerity::INVALID_OBJECT_ID;
    Celerity::ObjectId newParentId = Celerity::INVALID_OBJECT_ID;
};

struct SetLocalTransform final
{
    Celerity::ObjectId id = Celerity::INVALID_OBJECT_ID;
    bool logical = true;

    /// \warning Ignored if not ::logical.
    bool skipInterpolation = false;

    Math::Transform3d transform;
};

struct CheckTransform final
{
    Celerity::ObjectId id = Celerity::INVALID_OBJECT_ID;
    bool logical = true;
    bool local = true;

    /// \warning Ignored if ::local.
    bool useModifyQuery = false;

    Math::Transform3d expectedTransform;
};
} // namespace Requests

using Request = std::
    variant<Requests::CreateTransform, Requests::ChangeParent, Requests::SetLocalTransform, Requests::CheckTransform>;

namespace RequestExecutor
{
using RequestPacket = Container::Vector<Request>;

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                       Container::Vector<RequestPacket> _requests) noexcept;

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        Container::Vector<RequestPacket> _requests) noexcept;
} // namespace RequestExecutor
} // namespace Emergence::Transform::Test
