#include <thread>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

#include <Transform/Test/Task.hpp>

namespace Emergence::Transform::Test
{
using namespace Memory::Literals;

bool OperationsTestIncludeMarker () noexcept
{
    return true;
}

void OperationsTest (RequestExecutor::RequestPacket _scenario)
{
    Celerity::World world {"TestWorld"_us};
    Celerity::PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, Celerity::PipelineType::FIXED);
    RequestExecutor::AddToFixedUpdate (builder, {std::move (_scenario)});
    REQUIRE (builder.End (std::thread::hardware_concurrency ()));
    Celerity::WorldTestingUtility::RunFixedUpdateOnce (world);
}

void HierarchyTest (bool _logical, bool _useModifyQuery)
{
    using namespace Emergence::Math;
    using namespace Emergence::Transform::Test::Requests;

    const Transform3d expectedTransform3 {{9.0f, -4.0f, -2.0f}, Quaternion::IDENTITY, {2.0f, 6.0f, 4.0f}};
    const Transform3d expectedTransform3Changed {{3.0f, -6.0f, -6.0f}, Quaternion::IDENTITY, {2.0f, 2.0f, 4.0f}};
    const Transform3d expectedTransform3OtherParent {
        {12.0f, 4.0f, -12.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 6.0f}};
    const Transform3d expectedTransform5 {{15.0f, -38.0f, 33.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 3.0f}};

    OperationsTest ({
        CreateTransform {0u, Emergence::Celerity::INVALID_OBJECT_ID},
        SetLocalTransform {0u, _logical, false, {{2.0f, 4.0f, 1.0f}, Quaternion::IDENTITY}},

        CreateTransform {1u, 0u},
        SetLocalTransform {1u, _logical, false, {{1.0f, 0.0f, -1.0f}, Quaternion::IDENTITY, {2.0f, 2.0f, 2.0f}}},

        CreateTransform {2u, 1u},
        SetLocalTransform {2u, _logical, false, {{3.0f, 11.0f, 2.0f}, Quaternion::IDENTITY, {1.0f, 3.0f, 1.0f}}},

        CreateTransform {3u, 2u},
        SetLocalTransform {3u, _logical, false, {{0.0f, -5.0f, -3.0f}, Quaternion::IDENTITY, {1.0f, 1.0f, 2.0f}}},

        CreateTransform {4u, Emergence::Celerity::INVALID_OBJECT_ID},
        SetLocalTransform {4u, _logical, false, {{12.0f, -5.0f, 3.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 3.0f}}},

        CreateTransform {5u, 4u},
        SetLocalTransform {5u, _logical, false, {{1.0f, 10.0f, 11.0f}}},

        // Do several checks to test caching.
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3},
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3},
        CheckTransform {5u, _logical, false, _useModifyQuery, expectedTransform5},
        CheckTransform {5u, _logical, false, _useModifyQuery, expectedTransform5},

        // Check that caching reacts to change inside hierarchy correctly.
        SetLocalTransform {2u, _logical, false, {}},
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3Changed},
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3Changed},

        // Check that caching reacts to parent change correctly.
        ChangeParent {3u, 4u},
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3OtherParent},
        CheckTransform {3u, _logical, false, _useModifyQuery, expectedTransform3OtherParent},
    });
}
} // namespace Emergence::Transform::Test

using namespace Emergence::Transform::Test::Requests;
using namespace Emergence::Transform::Test;

BEGIN_SUITE (TransformOperations)

TEST_CASE (SetAndCheck)
{
    const Emergence::Math::Transform3d logicalTransform {
        {-11.0f, 15.0f, 23.7f}, {{0.66f, 0.57f, 1.59f}}, {1.0f, 2.0f, 3.1f}};
    const Emergence::Math::Transform3d visualTransform {
        {-11.33f, 15.127f, 23.34f}, {{0.69f, 0.51f, 1.33f}}, {2.0f, 1.0f, 4.2f}};

    OperationsTest ({
        CreateTransform {0u, Emergence::Celerity::INVALID_OBJECT_ID},
        SetLocalTransform {0u, true, false, logicalTransform},
        SetLocalTransform {0u, false, false, visualTransform},
        CheckTransform {0u, true, true, false, logicalTransform},
        CheckTransform {0u, true, false, false, logicalTransform},
        CheckTransform {0u, false, true, false, visualTransform},
        CheckTransform {0u, false, false, false, visualTransform},
    });
}

TEST_CASE (LogicalTransformHierarchy)
{
    HierarchyTest (true, false);
}

TEST_CASE (VisualTransformHierarchy)
{
    HierarchyTest (false, false);
}

TEST_CASE (WorldTransformUsingModifyQuery)
{
    HierarchyTest (true, true);
}

END_SUITE
