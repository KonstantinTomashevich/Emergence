#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(push)
// We do not care about excessive padding in test tasks. Also, we need to do it before variant declaration.
#    pragma warning(disable : 4324)
#endif

#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Math/Transform2d.hpp>
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
    SetLocalTransform (UniqueId _id,
                       bool _logical,
                       bool _skipInterpolation,
                       const Math::Transform2d &_transform2D) noexcept;

    SetLocalTransform (UniqueId _id,
                       bool _logical,
                       bool _skipInterpolation,
                       const Math::Transform3d &_transform3D) noexcept;

    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    bool logical = true;

    /// \warning Ignored if not ::logical.
    bool skipInterpolation = false;

    union
    {
        Math::Transform2d transform2d;
        Math::Transform3d transform3d;
    };
};

struct CheckTransform final
{
    CheckTransform (UniqueId _id,
                    bool _logical,
                    bool _local,
                    bool _useModifyQuery,
                    const Math::Transform2d &_expectedTransform2D) noexcept;

    CheckTransform (UniqueId _id,
                    bool _logical,
                    bool _local,
                    bool _useModifyQuery,
                    const Math::Transform3d &_expectedTransform3D) noexcept;

    Celerity::UniqueId id = Celerity::INVALID_UNIQUE_ID;
    bool logical = true;
    bool local = true;

    /// \warning Ignored if ::local.
    bool useModifyQuery = false;

    union
    {
        Math::Transform2d expectedTransform2d;
        Math::Transform3d expectedTransform3d;
    };
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

using Request = Container::Variant<Requests::CreateTransform,
                                   Requests::ChangeParent,
                                   Requests::SetLocalTransform,
                                   Requests::CheckTransform,
                                   Requests::CheckTransformExists,
                                   Requests::RemoveTransform>;

namespace RequestExecutor
{
using RequestPacket = Container::Vector<Request>;

void Add2dToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                         Container::Vector<RequestPacket> _requests,
                         bool _withHierarchyCleanup = false) noexcept;

void Add2dToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                          Container::Vector<RequestPacket> _requests) noexcept;

void Add3dToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                         Container::Vector<RequestPacket> _requests,
                         bool _withHierarchyCleanup = false) noexcept;

void Add3dToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                          Container::Vector<RequestPacket> _requests) noexcept;
} // namespace RequestExecutor
} // namespace Emergence::Celerity::Test

#if defined(_MSC_VER) && !defined(__clang__)
#    pragma warning(pop)
#endif
