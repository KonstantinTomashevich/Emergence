#include <algorithm>

#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;
using namespace Requests;

// Use 1 us fixed frames to make test time points more readable.
constexpr float TEST_FIXED_FRAME_TIME_S = 0.000001f;

template <bool use2d>
void SyncTest (Container::Vector<uint64_t> _timeSamples,
               Container::Vector<RequestExecutor::RequestPacket> _fixedRequests,
               Container::Vector<RequestExecutor::RequestPacket> _normalRequests)
{
    World world {"TestWorld"_us, WorldConfiguration {{TEST_FIXED_FRAME_TIME_S}}};
    PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    if (use2d)
    {
        RequestExecutor::Add2dToFixedUpdate (builder, std::move (_fixedRequests));
    }
    else
    {
        RequestExecutor::Add3dToFixedUpdate (builder, std::move (_fixedRequests));
    }

    REQUIRE (builder.End ());
    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);

    if (use2d)
    {
        VisualTransformSync::Add2dToNormalUpdate (builder);
        RequestExecutor::Add2dToNormalUpdate (builder, std::move (_normalRequests));
    }
    else
    {
        VisualTransformSync::Add3dToNormalUpdate (builder);
        RequestExecutor::Add3dToNormalUpdate (builder, std::move (_normalRequests));
    }

    REQUIRE (builder.End ());
    std::sort (_timeSamples.begin (), _timeSamples.end ());

    for (const uint64_t time : _timeSamples)
    {
        Time::Override (time);
        world.Update ();
    }
}

template <typename Transform>
void TrivialInterpolationTest (const Transform &_initialTransform,
                               const Transform &_transform025,
                               const Transform &_transform080,
                               const Transform &_targetTransform)
{
    SyncTest<std::is_same_v<Transform, Math::Transform2d>> (
        {
            0u,
            1000u,
            1250u,
            1800u,
            2200u,
        },
        {
            {
                CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
                SetLocalTransform {0u, true, true, _initialTransform},
            },
            {SetLocalTransform {0u, true, false, _targetTransform}},
        },
        {
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
            {{CheckTransform {0u, false, true, false, _transform025}}},
            {{CheckTransform {0u, false, true, false, _transform080}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
        });
}

template <typename Transform>
void InterpolationSkipTest (const Transform &_initialTransform, const Transform &_targetTransform)
{
    SyncTest<std::is_same_v<Transform, Math::Transform2d>> (
        {
            0u,
            1000u,
            1250u,
            1800u,
            2200u,
        },
        {
            {
                CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
                SetLocalTransform {0u, true, true, _initialTransform},
            },
            {SetLocalTransform {0u, true, true, _targetTransform}},
        },
        {
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
        });
}

template <typename Transform>
void InterpolationAndWorldTransformTest (const Transform &_parentInitialTransform,
                                         const Transform &_parentTargetTransform,
                                         const Transform &_childInitialTransform,
                                         const Transform &_childTargetTransform,
                                         const Transform &_childWorldTransform025,
                                         const Transform &_childWorldTransform080)
{
    SyncTest<std::is_same_v<Transform, Math::Transform2d>> (
        {
            0u,
            1000u,
            1250u,
            1800u,
            2200u,
        },
        {{
             {
                 CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
                 SetLocalTransform {0u, true, true, _parentInitialTransform},
                 CreateTransform {1u, 0u},
                 SetLocalTransform {1u, true, true, _childInitialTransform},
             },
         },
         {
             SetLocalTransform {0u, true, false, _parentTargetTransform},
             SetLocalTransform {1u, true, false, _childTargetTransform},
         }},
        {
            {{CheckTransform {1u, false, false, false, _parentInitialTransform * _childInitialTransform}}},
            {{CheckTransform {1u, false, false, false, _parentInitialTransform * _childInitialTransform}}},
            {{CheckTransform {1u, false, false, false, _childWorldTransform025}}},
            {{CheckTransform {1u, false, false, false, _childWorldTransform080}}},
            {{CheckTransform {1u, false, false, false, _parentTargetTransform * _childTargetTransform}}},
        });
}

template <typename Transform>
void InterpolationWithPauseTest (const Transform &_initialTransform,
                                 const Transform &_transform025,
                                 const Transform &_transform080,
                                 const Transform &_targetTransform)
{
    SyncTest<std::is_same_v<Transform, Math::Transform2d>> (
        {
            0u,
            1000u,
            1250u,
            1800u,
            2200u,
            4000u,
            4200u,
            4750u,
            5200u,
        },
        {
            {
                CreateTransform {0u, Emergence::Celerity::INVALID_UNIQUE_ID},
                SetLocalTransform {0u, true, true, _initialTransform},
            },
            {SetLocalTransform {0u, true, false, _targetTransform}},
            {},
            {SetLocalTransform {0u, true, false, _initialTransform}},
        },
        {
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
            {{CheckTransform {0u, false, true, false, _transform025}}},
            {{CheckTransform {0u, false, true, false, _transform080}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
            {{CheckTransform {0u, false, true, false, _targetTransform}}},
            {{CheckTransform {0u, false, true, false, _transform080}}},
            {{CheckTransform {0u, false, true, false, _transform025}}},
            {{CheckTransform {0u, false, true, false, _initialTransform}}},
        });
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Math;
using namespace Emergence::Celerity::Test;

BEGIN_SUITE (TransformVisualSync2d)

TEST_CASE (TrivialInterpolation)
{
    TrivialInterpolationTest<Transform2d> ({{1.0f, 3.0f}}, {{8.5f, 0.5f}, PI * 0.125f, {1.25f, 1.5f}},
                                           {{25.0f, -5.0f}, PI * 0.4f, {1.8f, 2.6f}},
                                           {{31.0f, -7.0f}, PI * 0.5f, {2.0f, 3.0f}});
}

TEST_CASE (InterpolationSkip)
{
    InterpolationSkipTest<Transform2d> ({{1.0f, 3.0f}}, {{31.0f, -7.0f}, PI * 0.5f, {2.0f, 3.0f}});
}

TEST_CASE (InterpolationAndWorldTransform)
{
    InterpolationAndWorldTransformTest<Transform2d> ({}, {{2.0f, 0.0f}}, {{-1.0f, 0.0f}}, {{1.0f, 0.0f}},
                                                     Transform2d {{0.5f, 0.0f}} * Transform2d {{-0.5f, 0.0f}},
                                                     Transform2d {{1.6f, 0.0f}} * Transform2d {{0.6f, 0.0f}});
}

TEST_CASE (InterpolationWithPause)
{
    InterpolationWithPauseTest<Transform2d> ({{2.0f, 4.0f}}, {{9.0f, 8.0f}}, {{24.4f, 16.8f}}, {{30.0f, 20.0f}});
}

END_SUITE

BEGIN_SUITE (TransformVisualSync3d)

TEST_CASE (TrivialInterpolation)
{
    TrivialInterpolationTest<Transform3d> (
        {{1.0f, 3.0f, 5.0f}},
        {{8.5f, 0.5f, 10.0f}, {0.183853373f, 0.0761545449f, 0.0761545449f, 0.97703582f}, {1.25f, 1.5f, 1.25f}},
        {{25.0f, -5.0f, 21.0f}, {0.547313631f, 0.226704717f, 0.226704717f, 0.77308315f}, {1.8f, 2.6f, 1.8f}},
        {{31.0f, -7.0f, 25.0f}, {{PI * 0.5f, PI * 0.25f, 0.0f}}, {2.0f, 3.0f, 2.0f}});
}

TEST_CASE (InterpolationSkip)
{
    InterpolationSkipTest<Transform3d> ({{1.0f, 3.0f, 5.0f}},
                                        {{31.0f, -7.0f, 25.0f}, {{PI * 0.5f, PI * 0.25f, 0.0f}}, {2.0f, 3.0f, 2.0f}});
}

TEST_CASE (InterpolationAndWorldTransform)
{
    InterpolationAndWorldTransformTest<Transform3d> (
        {}, {{2.0f, 0.0f, 2.0f}}, {{-1.0f, 0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}},
        Transform3d {{0.5f, 0.0f, 0.5f}} * Transform3d {{-0.5f, 0.0f, 0.0f}},
        Transform3d {{1.6f, 0.0f, 1.6f}} * Transform3d {{0.6f, 0.0f, 0.0f}});
}

TEST_CASE (InterpolationWithPause)
{
    InterpolationWithPauseTest<Transform3d> ({{2.0f, 4.0f, 10.0f}}, {{9.0f, 8.0f, 15.0f}}, {{24.4f, 16.8f, 26.0f}},
                                             {{30.0f, 20.0f, 30.0f}});
}

END_SUITE
