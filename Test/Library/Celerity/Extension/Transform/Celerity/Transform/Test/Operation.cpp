#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;

void OperationsTest (RequestExecutor::RequestPacket _scenario, bool _use2d)
{
    World world {"TestWorld"_us};
    PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    builder.AddCheckpoint (TransformHierarchyCleanup::Checkpoint::FINISHED);

    if (_use2d)
    {
        RequestExecutor::Add2dToFixedUpdate (builder, {std::move (_scenario)});
    }
    else
    {
        RequestExecutor::Add3dToFixedUpdate (builder, {std::move (_scenario)});
    }

    REQUIRE (builder.End ());
    WorldTestingUtility::RunFixedUpdateOnce (world);
}

template <typename Transform>
void HierarchyTest (bool _logical,
                    bool _useModifyQuery,
                    const Transform &_initial0,
                    const Transform &_initial1,
                    const Transform &_initial2,
                    const Transform &_initial3,
                    const Transform &_initial4,
                    const Transform &_initial5,
                    const Transform &_expected3,
                    const Transform &_expected3Changed,
                    const Transform &_expected3OtherParent,
                    const Transform &_expected5)
{
    using namespace Requests;

    OperationsTest (
        {
            CreateTransform {0u, INVALID_UNIQUE_ID},
            SetLocalTransform {0u, _logical, false, _initial0},

            CreateTransform {1u, 0u},
            SetLocalTransform {1u, _logical, false, _initial1},

            CreateTransform {2u, 1u},
            SetLocalTransform {2u, _logical, false, _initial2},

            CreateTransform {3u, 2u},
            SetLocalTransform {3u, _logical, false, _initial3},

            CreateTransform {4u, INVALID_UNIQUE_ID},
            SetLocalTransform {4u, _logical, false, _initial4},

            CreateTransform {5u, 4u},
            SetLocalTransform {5u, _logical, false, _initial5},

            // Do several checks to test caching.
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3},
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3},
            CheckTransform {5u, _logical, false, _useModifyQuery, _expected5},
            CheckTransform {5u, _logical, false, _useModifyQuery, _expected5},

            // Check that caching reacts to change inside hierarchy correctly.
            SetLocalTransform {2u, _logical, false, Transform {}},
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3Changed},
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3Changed},

            // Check that caching reacts to parent change correctly.
            ChangeParent {3u, 4u},
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3OtherParent},
            CheckTransform {3u, _logical, false, _useModifyQuery, _expected3OtherParent},
        },
        std::is_same_v<Transform, Math::Transform2d>);
}

void HierarchyTest3d (bool _logical, bool _useModifyQuery)
{
    using namespace Math;

    HierarchyTest<Transform3d> (_logical, _useModifyQuery, {{2.0f, 4.0f, 1.0f}, Quaternion::IDENTITY},
                                {{1.0f, 0.0f, -1.0f}, Quaternion::IDENTITY, {2.0f, 2.0f, 2.0f}},
                                {{3.0f, 11.0f, 2.0f}, Quaternion::IDENTITY, {1.0f, 3.0f, 1.0f}},
                                {{0.0f, -5.0f, -3.0f}, Quaternion::IDENTITY, {1.0f, 1.0f, 2.0f}},
                                {{12.0f, -5.0f, 3.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 3.0f}},
                                {{1.0f, 10.0f, 11.0f}},
                                {{9.0f, -4.0f, -2.0f}, Quaternion::IDENTITY, {2.0f, 6.0f, 4.0f}},
                                {{3.0f, -6.0f, -6.0f}, Quaternion::IDENTITY, {2.0f, 2.0f, 4.0f}},
                                {{12.0f, 4.0f, -12.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 6.0f}},
                                {{15.0f, -38.0f, 33.0f}, {{PI * 0.5f, 0.0f, 0.0f}}, {3.0f, 3.0f, 3.0f}});
}

void HierarchyTest2d (bool _logical, bool _useModifyQuery)
{
    using namespace Math;

    HierarchyTest<Transform2d> (_logical, _useModifyQuery, {{2.0f, 4.0f}}, {{1.0f, 0.0f}, 0.0f, {2.0f, 2.0f}},
                                {{3.0f, 11.0f}, 0.0f, {1.0f, 3.0f}}, {{0.0f, -5.0f}, 0.0f, {1.0f, 1.0f}},
                                {{12.0f, -5.0f}, PI * 0.5f, {3.0f, 3.0f}}, {{1.0f, 10.0f}},
                                {{9.0f, -4.0f}, 0.0f, {2.0f, 6.0f}}, {{3.0f, -6.0f}, 0.0f, {2.0f, 2.0f}},
                                {{27.0f, -5.0f}, PI * 0.5f, {3.0f, 3.0f}}, {{-18.0f, -2.0f}, PI * 0.5f, {3.0f, 3.0f}});
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity::Test::Requests;
using namespace Emergence::Celerity::Test;

BEGIN_SUITE (TransformOperations2d)

TEST_CASE (SetAndCheck)
{
    const Emergence::Math::Transform2d logicalTransform {{-11.0f, 15.0f}, 0.66f, {1.0f, 2.0f}};
    const Emergence::Math::Transform2d visualTransform {{-11.33f, 15.127f}, 0.69f, {2.0f, 1.0f}};

    OperationsTest (
        {
            CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
            SetLocalTransform {0u, true, false, logicalTransform},
            SetLocalTransform {0u, false, false, visualTransform},
            CheckTransform {0u, true, true, false, logicalTransform},
            CheckTransform {0u, true, false, false, logicalTransform},
            CheckTransform {0u, false, true, false, visualTransform},
            CheckTransform {0u, false, false, false, visualTransform},
        },
        true);
}

TEST_CASE (LogicalTransformHierarchy)
{
    HierarchyTest2d (true, false);
}

TEST_CASE (VisualTransformHierarchy)
{
    HierarchyTest2d (false, false);
}

TEST_CASE (WorldTransformUsingModifyQuery)
{
    HierarchyTest2d (true, true);
}

END_SUITE

BEGIN_SUITE (TransformOperations3d)

TEST_CASE (SetAndCheck)
{
    const Emergence::Math::Transform3d logicalTransform {
        {-11.0f, 15.0f, 23.7f}, {{0.66f, 0.57f, 1.59f}}, {1.0f, 2.0f, 3.1f}};
    const Emergence::Math::Transform3d visualTransform {
        {-11.33f, 15.127f, 23.34f}, {{0.69f, 0.51f, 1.33f}}, {2.0f, 1.0f, 4.2f}};

    OperationsTest (
        {
            CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
            SetLocalTransform {0u, true, false, logicalTransform},
            SetLocalTransform {0u, false, false, visualTransform},
            CheckTransform {0u, true, true, false, logicalTransform},
            CheckTransform {0u, true, false, false, logicalTransform},
            CheckTransform {0u, false, true, false, visualTransform},
            CheckTransform {0u, false, false, false, visualTransform},
        },
        false);
}

TEST_CASE (LogicalTransformHierarchy)
{
    HierarchyTest3d (true, false);
}

TEST_CASE (VisualTransformHierarchy)
{
    HierarchyTest3d (false, false);
}

TEST_CASE (WorldTransformUsingModifyQuery)
{
    HierarchyTest3d (true, true);
}

END_SUITE
