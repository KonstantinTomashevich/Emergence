#include <API/Common/MuteWarnings.hpp>

BEGIN_MUTING_PADDING_WARNING

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;
using namespace Requests;

void HierarchyCleanupTest (Container::Vector<RequestExecutor::RequestPacket> _scenario, bool _fixed, bool _use2d)
{
    World world {"TestWorld"_us};
    {
        EventRegistrar registrar {&world};
        if (_use2d)
        {
            RegisterTransform2dEvents (registrar);
        }
        else
        {
            RegisterTransform3dEvents (registrar);
        }

        RegisterTransformCommonEvents (registrar);
    }

    PipelineBuilder builder {world.GetRootView ()};
    const std::size_t stepCount = _scenario.size ();

    if (_fixed)
    {
        builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
        builder.AddCheckpoint (TransformVisualSync::Checkpoint::STARTED);

        if (_use2d)
        {
            TransformHierarchyCleanup::Add2dToFixedUpdate (builder);
            RequestExecutor::Add2dToFixedUpdate (builder, std::move (_scenario));
        }
        else
        {
            TransformHierarchyCleanup::Add3dToFixedUpdate (builder);
            RequestExecutor::Add3dToFixedUpdate (builder, std::move (_scenario));
        }
    }
    else
    {
        builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
        builder.AddCheckpoint (TransformVisualSync::Checkpoint::STARTED);
        builder.AddCheckpoint (TransformVisualSync::Checkpoint::FINISHED);

        if (_use2d)
        {
            TransformHierarchyCleanup::Add2dToNormalUpdate (builder);
            RequestExecutor::Add2dToNormalUpdate (builder, std::move (_scenario));
        }
        else
        {
            TransformHierarchyCleanup::Add3dToNormalUpdate (builder);
            RequestExecutor::Add3dToNormalUpdate (builder, std::move (_scenario));
        }
    }

    REQUIRE (builder.End ());

    for (std::size_t index = 0u; index < stepCount; ++index)
    {
        if (_fixed)
        {
            WorldTestingUtility::RunFixedUpdateOnce (world);
        }
        else
        {
            WorldTestingUtility::RunNormalUpdateOnce (world, 16000000u);
        }
    }
}

static const Container::Vector<RequestExecutor::RequestPacket> ONE_LEVEL_TEST {
    {
        CreateTransform {0u, INVALID_UNIQUE_ID},
        CreateTransform {1u, 0u},
        CreateTransform {2u, 0u},
        CreateTransform {3u, INVALID_UNIQUE_ID},
    },
    {
        CheckTransformExists {0u, true},
        CheckTransformExists {1u, true},
        CheckTransformExists {2u, true},
        CheckTransformExists {3u, true},
        RemoveTransform {0u},
    },
    {
        CheckTransformExists {0u, false},
        CheckTransformExists {1u, false},
        CheckTransformExists {2u, false},
        CheckTransformExists {3u, true},
    }};

static const Container::Vector<RequestExecutor::RequestPacket> MULTIPLE_LEVELS_TEST {
    {
        CreateTransform {0u, INVALID_UNIQUE_ID},
        CreateTransform {1u, 0u},
        CreateTransform {2u, 1u},
        CreateTransform {3u, 2u},
        CreateTransform {4u, INVALID_UNIQUE_ID},
    },
    {
        CheckTransformExists {0u, true},
        CheckTransformExists {1u, true},
        CheckTransformExists {2u, true},
        CheckTransformExists {3u, true},
        CheckTransformExists {4u, true},
        RemoveTransform {0u},
    },
    {
        CheckTransformExists {0u, false},
        CheckTransformExists {1u, false},
        CheckTransformExists {2u, false},
        CheckTransformExists {3u, false},
        CheckTransformExists {4u, true},
    }};

static const Container::Vector<RequestExecutor::RequestPacket> INTERMEDIATE_TRANSFORM_REMOVAL_TEST {
    {
        CreateTransform {0u, INVALID_UNIQUE_ID},
        CreateTransform {1u, 0u},
        CreateTransform {2u, 1u},
        CreateTransform {3u, 2u},
        CreateTransform {4u, INVALID_UNIQUE_ID},
    },
    {
        CheckTransformExists {0u, true},
        CheckTransformExists {1u, true},
        CheckTransformExists {2u, true},
        CheckTransformExists {3u, true},
        CheckTransformExists {4u, true},
        RemoveTransform {2u},
    },
    {
        CheckTransformExists {0u, true},
        CheckTransformExists {1u, true},
        CheckTransformExists {2u, false},
        CheckTransformExists {3u, false},
        CheckTransformExists {4u, true},
    }};
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity::Test::Requests;

BEGIN_SUITE (HierarchyCleanupFixed2d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, true, true);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, true, true);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, true, true);
}

END_SUITE

BEGIN_SUITE (HierarchyCleanupNormal2d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, false, true);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, false, true);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, false, true);
}

END_SUITE

BEGIN_SUITE (HierarchyCleanupFixed3d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, true, false);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, true, false);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, true, false);
}

END_SUITE

BEGIN_SUITE (HierarchyCleanupNormal3d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, false, false);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, false, false);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, false, false);
}

END_SUITE

END_MUTING_WARNINGS
