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

void HierarchyCleanupTest (Container::Vector<RequestExecutor::RequestPacket> _scenario, bool _use2d)
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

    PipelineBuilder builder {&world};
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    const std::size_t stepCount = _scenario.size ();
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

    REQUIRE (builder.End ());

    for (std::size_t index = 0u; index < stepCount; ++index)
    {
        WorldTestingUtility::RunFixedUpdateOnce (world);
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

BEGIN_SUITE (HierarchyCleanup2d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, true);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, true);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, true);
}

END_SUITE

BEGIN_SUITE (HierarchyCleanup3d)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest (ONE_LEVEL_TEST, false);
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest (MULTIPLE_LEVELS_TEST, false);
}

TEST_CASE (IntermediateTransformRemoval)
{
    HierarchyCleanupTest (INTERMEDIATE_TRANSFORM_REMOVAL_TEST, false);
}

END_SUITE
