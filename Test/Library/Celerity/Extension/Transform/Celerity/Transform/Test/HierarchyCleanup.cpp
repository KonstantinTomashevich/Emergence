#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Test/Task.hpp>
#include <Celerity/Transform/Transform3dHierarchyCleanup.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Memory::Literals;

void HierarchyCleanupTest (Container::Vector<RequestExecutor::RequestPacket> _scenario)
{
    World world {"TestWorld"_us};
    {
        EventRegistrar registrar {&world};
        RegisterTransformEvents (registrar);
    }

    PipelineBuilder builder {&world};
    builder.Begin ("FixedUpdate"_us, PipelineType::FIXED);
    HierarchyCleanup::AddToFixedUpdate (builder);
    const std::size_t stepCount = _scenario.size ();
    RequestExecutor::AddToFixedUpdate (builder, std::move (_scenario));
    REQUIRE (builder.End ());

    for (std::size_t index = 0u; index < stepCount; ++index)
    {
        WorldTestingUtility::RunFixedUpdateOnce (world);
    }
}
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity::Test::Requests;

BEGIN_SUITE (HierarchyCleanup)

TEST_CASE (OneLevel)
{
    HierarchyCleanupTest ({{
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
                           }});
}

TEST_CASE (MultipleLevels)
{
    HierarchyCleanupTest ({{
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
                           }});
}

END_SUITE
