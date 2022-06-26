#include <algorithm>
#include <thread>

#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>

#include <Math/Constants.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;

// Use 1 us fixed frames to make test time points more readable.
constexpr float TEST_FIXED_FRAME_TIME_S = 0.000001f;

void SyncTest (Container::Vector<uint64_t> _timeSamples,
               Container::Vector<RequestExecutor::RequestPacket> _fixedRequests,
               Container::Vector<RequestExecutor::RequestPacket> _normalRequests)
{
    World world {"TestWorld"_us, WorldConfiguration {{TEST_FIXED_FRAME_TIME_S}}};
    PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    RequestExecutor::AddToFixedUpdate (builder, std::move (_fixedRequests));
    REQUIRE (builder.End (std::thread::hardware_concurrency ()));

    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    builder.AddCheckpoint (VisualTransformSync::Checkpoint::SYNC_FINISHED);
    VisualTransformSync::AddToNormalUpdate (builder);
    RequestExecutor::AddToNormalUpdate (builder, std::move (_normalRequests));
    REQUIRE (builder.End (std::thread::hardware_concurrency ()));

    std::sort (_timeSamples.begin (), _timeSamples.end ());
    for (const uint64_t time : _timeSamples)
    {
        Time::Override (time);
        world.Update ();
    }
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Math;
using namespace Emergence::Celerity::Test::Requests;
using namespace Emergence::Celerity::Test;

BEGIN_SUITE (TransformVisualSync)

TEST_CASE (TrivialInterpolation)
{
    const Transform3d initialTransform {{1.0f, 3.0f, 5.0f}};
    const Transform3d targetTransform {{31.0f, -7.0f, 25.0f}, {{PI * 0.5f, PI * 0.25f, 0.0f}}, {2.0f, 3.0f, 2.0f}};

    SyncTest (
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
                SetLocalTransform {0u, true, true, initialTransform},
            },
            {SetLocalTransform {0u, true, false, targetTransform}},
        },
        {
            {{CheckTransform {0u, false, true, false, initialTransform}}},
            {{CheckTransform {0u, false, true, false, initialTransform}}},
            {{CheckTransform {0u,
                              false,
                              true,
                              false,
                              {{8.5f, 0.5f, 10.0f},
                               {0.183853373f, 0.0761545449f, 0.0761545449f, 0.97703582f},
                               {1.25f, 1.5f, 1.25f}}}}},
            {{CheckTransform {
                0u,
                false,
                true,
                false,
                {{25.0f, -5.0f, 21.0f}, {0.547313631f, 0.226704717f, 0.226704717f, 0.77308315f}, {1.8f, 2.6f, 1.8f}}}}},
            {{CheckTransform {0u, false, true, false, targetTransform}}},
        });
}

TEST_CASE (InterpolationSkip)
{
    const Transform3d initialTransform {{1.0f, 3.0f, 5.0f}};
    const Transform3d targetTransform {{31.0f, -7.0f, 25.0f}, {{PI * 0.5f, PI * 0.25f, 0.0f}}, {2.0f, 3.0f, 2.0f}};

    SyncTest (
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
                SetLocalTransform {0u, true, true, initialTransform},
            },
            {SetLocalTransform {0u, true, true, targetTransform}},
        },
        {
            {{CheckTransform {0u, false, true, false, initialTransform}}},
            {{CheckTransform {0u, false, true, false, targetTransform}}},
            {{CheckTransform {0u, false, true, false, targetTransform}}},
            {{CheckTransform {0u, false, true, false, targetTransform}}},
            {{CheckTransform {0u, false, true, false, targetTransform}}},
        });
}

TEST_CASE (InterpolationAndWorldTransform)
{
    const Transform3d parentInitialTransform;
    const Transform3d parentTargetTransform {{2.0f, 0.0f, 2.0f}};

    const Transform3d childInitialTransform {{-1.0f, 0.0f, 0.0f}};
    const Transform3d childTargetTransform {{1.0f, 0.0f, 0.0f}};

    SyncTest (
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
                 SetLocalTransform {0u, true, true, parentInitialTransform},
                 CreateTransform {1u, 0u},
                 SetLocalTransform {1u, true, true, childInitialTransform},
             },
         },
         {
             SetLocalTransform {0u, true, false, parentTargetTransform},
             SetLocalTransform {1u, true, false, childTargetTransform},
         }},
        {
            {{CheckTransform {1u, false, false, false, parentInitialTransform * childInitialTransform}}},
            {{CheckTransform {1u, false, false, false, parentInitialTransform * childInitialTransform}}},
            {{CheckTransform {1u, false, false, false,
                              Transform3d {{0.5f, 0.0f, 0.5f}} * Transform3d {{-0.5f, 0.0f, 0.0f}}}}},
            {{CheckTransform {1u, false, false, false,
                              Transform3d {{1.6f, 0.0f, 1.6f}} * Transform3d {{0.6f, 0.0f, 0.0f}}}}},
            {{CheckTransform {1u, false, false, false, parentTargetTransform * childTargetTransform}}},
        });
}

END_SUITE
