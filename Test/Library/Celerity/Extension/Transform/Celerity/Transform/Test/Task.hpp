#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(push)
// We do not care about excessive padding in test tasks. Also, we need to do it before variant declaration.
#    pragma warning(disable : 4324)
#endif

#include <variant>

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Vector.hpp>

#include <Math/Transform3d.hpp>

namespace Emergence::Celerity::Test
{
namespace Requests
{
struct CreateTransform final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    Celerity::UniqueId parentId = Celerity::INVALID_UNIQUE_ID;
};

struct ChangeParent final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    Celerity::UniqueId newParentId = Celerity::INVALID_UNIQUE_ID;
};

struct SetLocalTransform final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    bool logical = true;

    /// \warning Ignored if not ::logical.
    bool skipInterpolation = false;

    Math::Transform3d transform;
};

struct CheckTransform final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    bool logical = true;
    bool local = true;

    /// \warning Ignored if ::local.
    bool useModifyQuery = false;

    Math::Transform3d expectedTransform;
};

struct CheckTransformExists final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    bool exists = true;
};

struct RemoveTransform final
{
    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
};
} // namespace Requests

using Request = std::variant<Requests::CreateTransform,
                             Requests::ChangeParent,
                             Requests::SetLocalTransform,
                             Requests::CheckTransform,
                             Requests::CheckTransformExists,
                             Requests::RemoveTransform>;

namespace RequestExecutor
{
using RequestPacket = Container::Vector<Request>;

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                       Container::Vector<RequestPacket> _requests) noexcept;

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        Container::Vector<RequestPacket> _requests) noexcept;
} // namespace RequestExecutor
} // namespace Emergence::Celerity::Test

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(pop)
#endif
