#include <algorithm>
#include <cstdint>
#include <thread>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Emergence::Memory::Literals;

bool EventRoutingTestIncludeMarker () noexcept
{
    return true;
}

namespace
{
EMERGENCE_CELERITY_EVENT1_DECLARATION (TestEvent, uint64_t, data);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TestEvent, REGULAR, data)

using EventPlan = Container::Vector<Container::Vector<uint64_t>>;

class EventProducer final : public TaskExecutorBase<EventProducer>
{
public:
    EventProducer (TaskConstructor &_constructor, EventPlan _productionPlan) noexcept;

    void Execute () noexcept;

private:
    InsertShortTermQuery insertEvents;
    std::size_t executionIndex = 0u;
    EventPlan productionPlan;
};

Emergence::Celerity::Test::EventProducer::EventProducer (TaskConstructor &_constructor,
                                                         EventPlan _productionPlan) noexcept
    : insertEvents (INSERT_SHORT_TERM (TestEvent)),
      productionPlan (std::move (_productionPlan))
{
}

void Emergence::Celerity::Test::EventProducer::Execute () noexcept
{
    REQUIRE (executionIndex < productionPlan.size ());
    auto cursor = insertEvents.Execute ();

    for (uint64_t data : productionPlan[executionIndex])
    {
        auto *event = static_cast<TestEvent *> (++cursor);
        event->data = data;
    }

    ++executionIndex;
}

class EventConsumer final : public TaskExecutorBase<EventConsumer>
{
public:
    EventConsumer (TaskConstructor &_constructor, EventPlan _consumptionPlan) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchEvents;
    std::size_t executionIndex = 0u;
    EventPlan consumptionPlan;
};

Emergence::Celerity::Test::EventConsumer::EventConsumer (TaskConstructor &_constructor,
                                                         EventPlan _consumptionPlan) noexcept
    : fetchEvents (FETCH_SEQUENCE (TestEvent)),
      consumptionPlan (std::move (_consumptionPlan))
{
    for (auto &framePlan : consumptionPlan)
    {
        std::sort (framePlan.begin (), framePlan.end ());
    }
}

void Emergence::Celerity::Test::EventConsumer::Execute () noexcept
{
    REQUIRE (executionIndex < consumptionPlan.size ());

    // Generally, event order is not guaranteed, therefore we collect all data and sort it before comparison.
    Container::Vector<uint64_t> fetchedData;

    auto cursor = fetchEvents.Execute ();
    while (const auto *event = static_cast<const TestEvent *> (*cursor))
    {
        fetchedData.emplace_back (event->data);
        ++cursor;
    }

    std::sort (fetchedData.begin (), fetchedData.end ());
    CHECK_EQUAL (fetchedData.size (), consumptionPlan[executionIndex].size ());
    const uint64_t amountToCheck = std::min (fetchedData.size (), consumptionPlan[executionIndex].size ());

    for (std::size_t index = 0u; index < amountToCheck; ++index)
    {
        CHECK_EQUAL (fetchedData[index], consumptionPlan[executionIndex][index]);
    }

    ++executionIndex;
}

template <typename Task>
void AddTestTask (PipelineBuilder &_builder,
                  Memory::UniqueString _name,
                  EventPlan _plan,
                  const Container::Vector<Memory::UniqueString> &_dependencies = {})
{
    TaskConstructor constructor = _builder.AddTask (_name);
    constructor.SetExecutor<Task> (std::move (_plan));

    for (Memory::UniqueString dependency : _dependencies)
    {
        if (*dependency)
        {
            constructor.DependOn (dependency);
        }
    }
}

void RegisterTestEvent (World *_world, EventRoute _route)
{
    EventRegistrar registrar {_world};
    registrar.CustomEvent ({TestEvent::Reflect ().mapping, _route});
}

void SimpleConsumptionTest (EventRoute _route, const EventPlan &_plan, bool _previousFrameMode)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, _route);

    EventPlan productionPlan = _plan;
    EventPlan consumptionPlan = _plan;

    // If we are testing previous frame events consumption, we need to add padding to each plan.
    if (_previousFrameMode)
    {
        productionPlan.emplace_back ();
        consumptionPlan.emplace (consumptionPlan.begin ());
    }

    const uint64_t steps = productionPlan.size ();

    PipelineBuilder builder {&world};
    REQUIRE (GetEventProducingPipeline (_route) == GetEventConsumingPipeline (_route));
    builder.Begin ("Update"_us, GetEventProducingPipeline (_route));

    const Emergence::Memory::UniqueString producerTask {"EventProducer"};
    const Emergence::Memory::UniqueString consumerTask {"EventConsumer"};

    AddTestTask<EventProducer> (builder, producerTask, std::move (productionPlan),
                                {_previousFrameMode ? consumerTask : ""_us});
    AddTestTask<EventConsumer> (builder, consumerTask, std::move (consumptionPlan),
                                {_previousFrameMode ? ""_us : producerTask});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());

    if (_previousFrameMode && _route == EventRoute::CUSTOM)
    {
        // There is no previous frame mode for custom pipelines, therefore pipeline should not be constructed.
        CHECK_EQUAL (pipeline, nullptr);
        return;
    }

    for (std::size_t index = 0u; index < steps; ++index)
    {
        // We don't care about time here, therefore we can directly call our pipeline.
        pipeline->Execute ();
    }
}
} // namespace
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test;

BEGIN_SUITE (EventRouting)

TEST_CASE (FixedCurrentFrame)
{
    SimpleConsumptionTest (EventRoute::FIXED, {{13u, 7u, 99u}, {7u}, {}, {1u}}, false);
}

TEST_CASE (FixedPreviousFrame)
{
    SimpleConsumptionTest (EventRoute::FIXED, {{13u, 7u, 99u}, {7u}, {}, {1u}}, true);
}

TEST_CASE (NormalCurrentFrame)
{
    SimpleConsumptionTest (EventRoute::NORMAL, {{13u, 7u, 99u}, {7u}, {}, {1u}}, false);
}

TEST_CASE (NormalPreviousFrame)
{
    SimpleConsumptionTest (EventRoute::NORMAL, {{13u, 7u, 99u}, {7u}, {}, {1u}}, true);
}

TEST_CASE (CustomCurrentFrame)
{
    SimpleConsumptionTest (EventRoute::CUSTOM, {{13u, 7u, 99u}, {7u}, {}, {1u}}, false);
}

TEST_CASE (CustomPreviousFrame)
{
    SimpleConsumptionTest (EventRoute::CUSTOM, {{13u, 7u, 99u}, {7u}, {}, {1u}}, true);
}

TEST_CASE (FixedToNormal)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FROM_FIXED_TO_NORMAL);

    // We are testing event accumulation too, therefore plans are so different.

    EventPlan productionPlan {{7u, 12u}, {13u, 7u, 11u}, {9u}, {42u}, {}};
    EventPlan consumptionPlan {
        // One fixed update.
        {7u, 12u},
        // Two fixed updates.
        {13u, 7u, 11u, 9u},
        // No fixed updates.
        {},
        // One fixed update.
        {42u},
        // No-event fixed update.
        {},
    };

    PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    AddTestTask<EventProducer> (builder, "EventProducer"_us, std::move (productionPlan));
    Pipeline *fixedPipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (fixedPipeline);

    builder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    AddTestTask<EventConsumer> (builder, "EventConsumer"_us, std::move (consumptionPlan));
    Pipeline *normalPipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (normalPipeline);

    // Pipelines are executed according to scenario, written above in consumption plan.
    // We don't care about time here, therefore we can directly call our pipeline.

    fixedPipeline->Execute ();
    normalPipeline->Execute ();

    fixedPipeline->Execute ();
    fixedPipeline->Execute ();
    normalPipeline->Execute ();

    normalPipeline->Execute ();

    fixedPipeline->Execute ();
    normalPipeline->Execute ();

    fixedPipeline->Execute ();
    normalPipeline->Execute ();
}

TEST_CASE (MultipleProducers)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FIXED);

    EventPlan firstProducerPlan {{3u, 7u}, {}, {13u}};
    EventPlan secondProducerPlan {{15u, 7u}, {21u}, {}};
    EventPlan consumerPlan {{3u, 7u, 7u, 15u}, {21u}, {13u}};
    const std::size_t steps = firstProducerPlan.size ();

    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const Emergence::Memory::UniqueString firstProducer {"FirstEventProducer"};
    const Emergence::Memory::UniqueString secondProducer {"SecondEventProducer"};

    AddTestTask<EventProducer> (builder, firstProducer, std::move (firstProducerPlan));
    AddTestTask<EventProducer> (builder, secondProducer, std::move (secondProducerPlan), {firstProducer});
    AddTestTask<EventConsumer> (builder, "EventConsumer"_us, std::move (consumerPlan), {firstProducer, secondProducer});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (pipeline);

    for (std::size_t index = 0u; index < steps; ++index)
    {
        pipeline->Execute ();
    }
}

TEST_CASE (MultipleConsumers)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FIXED);

    const EventPlan plan {{3u, 7u, 21u}, {73u, 42u}, {}, {13u, 10u}};
    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const Emergence::Memory::UniqueString producer {"EventProducer"};

    AddTestTask<EventProducer> (builder, producer, plan);
    AddTestTask<EventConsumer> (builder, "FirstConsumer"_us, plan, {producer});
    AddTestTask<EventConsumer> (builder, "SecondConsumer"_us, plan, {producer});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (pipeline);

    for (std::size_t index = 0u; index < plan.size (); ++index)
    {
        pipeline->Execute ();
    }
}

TEST_CASE (ConsumerDoesNotDependOnAllProducers)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FIXED);
    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const Emergence::Memory::UniqueString firstProducer {"FirstEventProducer"};
    const Emergence::Memory::UniqueString secondProducer {"SecondEventProducer"};

    AddTestTask<EventProducer> (builder, firstProducer, {});
    AddTestTask<EventProducer> (builder, secondProducer, {}, {firstProducer});
    AddTestTask<EventConsumer> (builder, "EventConsumer"_us, {}, {firstProducer});
    CHECK_EQUAL (builder.End (std::thread::hardware_concurrency ()), nullptr);
}

TEST_CASE (ConsumerIsNotDependencyOfAllProducers)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FIXED);
    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const Emergence::Memory::UniqueString firstProducer {"FirstProducer"};
    const Emergence::Memory::UniqueString consumer {"EventConsumer"};

    AddTestTask<EventConsumer> (builder, "EventConsumer"_us, {});
    AddTestTask<EventProducer> (builder, firstProducer, {});
    AddTestTask<EventProducer> (builder, "SecondProducer"_us, {}, {firstProducer, consumer});
    CHECK_EQUAL (builder.End (std::thread::hardware_concurrency ()), nullptr);
}

TEST_CASE (ConsumerIsBetweenProducers)
{
    World world {"TestWorld"_us};
    RegisterTestEvent (&world, EventRoute::FIXED);
    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const Emergence::Memory::UniqueString firstProducer {"FirstEventProducer"};
    const Emergence::Memory::UniqueString secondProducer {"SecondEventProducer"};
    const Emergence::Memory::UniqueString consumer {"EventConsumer"};

    AddTestTask<EventProducer> (builder, firstProducer, {});
    AddTestTask<EventConsumer> (builder, consumer, {}, {firstProducer});
    AddTestTask<EventProducer> (builder, secondProducer, {}, {consumer});
    CHECK_EQUAL (builder.End (std::thread::hardware_concurrency ()), nullptr);
}

END_SUITE
