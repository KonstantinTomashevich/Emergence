#include <cstdint>
#include <thread>
#include <variant>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/Test/EventTriggering.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Emergence::Memory::Literals;

bool EventTriggeringTestIncludeMarker () noexcept
{
    return true;
}

namespace
{
struct TestRecord final
{
    uint64_t id = 0u;
    float health = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    [[nodiscard]] Container::String ToString () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId angle;
        StandardLayout::FieldId health;
        Emergence::StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};

Container::String Emergence::Celerity::Test::TestRecord::ToString () const noexcept
{
    return EMERGENCE_BUILD_STRING ("{ id = ", id, ", health = ", health, ", x = ", x, ", y = ", y, ", angle = ", angle,
                                   " }");
}

TestRecord::Reflection &TestRecord::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestRecord)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTER_REGULAR (x)
        EMERGENCE_MAPPING_REGISTER_REGULAR (y)
        EMERGENCE_MAPPING_REGISTER_REGULAR (angle)
        EMERGENCE_MAPPING_REGISTER_REGULAR (health)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

struct TestRecordAddedEvent final
{
    uint64_t id = 0u;

    bool operator== (const TestRecordAddedEvent &_other) const = default;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        Emergence::StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};

TestRecordAddedEvent::Reflection &TestRecordAddedEvent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestRecordAddedEvent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

// In a lot of cases we need to know about addition in several pipelines (fixed and normal, for example).
// In order to do it, we need to have two event types: one per pipeline.
// We're using this event type in pair with TestRecordAddedEvent to check that this behaviour works correctly.
struct TestRecordAddedSharedEvent final
{
    uint64_t id = 0u;
    float health = 0.0f;

    bool operator== (const TestRecordAddedSharedEvent &_other) const = default;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId health;
        Emergence::StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};

TestRecordAddedSharedEvent::Reflection &TestRecordAddedSharedEvent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestRecordAddedSharedEvent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTER_REGULAR (health)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

struct TestRecordRemovedEvent final
{
    uint64_t id = 0u;
    // Intentionally add all fields except health in order to check multiblock copy out.
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;

    bool operator== (const TestRecordRemovedEvent &_other) const = default;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId angle;
        Emergence::StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};

TestRecordRemovedEvent::Reflection &TestRecordRemovedEvent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestRecordRemovedEvent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTER_REGULAR (x)
        EMERGENCE_MAPPING_REGISTER_REGULAR (y)
        EMERGENCE_MAPPING_REGISTER_REGULAR (angle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

// TODO: Add on change event triggering test.

namespace Tasks
{
struct AddRecord final
{
    TestRecord record;
};

struct RemoveRecord final
{
    uint64_t recordId;
};

using Task = std::variant<Tasks::AddRecord, Tasks::RemoveRecord>;

class Executor final : public TaskExecutorBase<Executor>
{
public:
    Executor (TaskConstructor &_constructor, Container::Vector<Task> _tasks) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertRecord;
    ModifyValueQuery modifyRecordById;
    Container::Vector<Task> tasks;
};

Executor::Executor (TaskConstructor &_constructor, Container::Vector<Task> _tasks) noexcept
    : insertRecord (_constructor.InsertLongTerm (TestRecord::Reflect ().mapping)),
      modifyRecordById (_constructor.ModifyValue (TestRecord::Reflect ().mapping, {TestRecord::Reflect ().id})),
      tasks (std::move (_tasks))
{
}

void Executor::Execute () noexcept
{
    for (const Task &task : tasks)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Tasks::AddRecord, Type>)
                {
                    LOG ("Adding record ", _task.record.ToString (), ".");
                    auto cursor = insertRecord.Execute ();
                    *static_cast<TestRecord *> (++cursor) = _task.record;
                }
                else if constexpr (std::is_same_v<Tasks::RemoveRecord, Type>)
                {
                    LOG ("Removing record with id ", _task.recordId, ".");
                    auto cursor = modifyRecordById.Execute (&_task.recordId);

                    if (*cursor)
                    {
                        ~cursor;
                        CHECK_WITH_MESSAGE (!*cursor, "Record ids are expected to be unique.");
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find record with id ", _task.recordId, ".");
                    }
                }
            },
            task);
    }
}

const Memory::UniqueString EXECUTOR_TASK_NAME {"Executor"};

void AddExecutorTask (PipelineBuilder &_builder, Container::Vector<Task> _tasks)
{
    TaskConstructor constructor = _builder.AddTask (EXECUTOR_TASK_NAME);
    constructor.SetExecutor<Executor> (std::move (_tasks));
}

template <typename EventType>
class Validator final : public TaskExecutorBase<Validator<EventType>>
{
public:
    Validator (TaskConstructor &_constructor, Container::Vector<EventType> _expected) noexcept;

    void Execute ();

private:
    FetchSequenceQuery fetch;
    Container::Vector<EventType> expected;
};

template <typename EventType>
Validator<EventType>::Validator (TaskConstructor &_constructor, Container::Vector<EventType> _expected) noexcept
    : fetch (_constructor.FetchSequence (EventType::Reflect ().mapping)),
      expected (std::move (_expected))
{
}

template <typename EventType>
void Validator<EventType>::Execute ()
{
    // Events order is not guaranteed, therefore we need to fetch all events and compare vectors.
    Container::Vector<const EventType *> found;

    auto cursor = fetch.Execute ();
    while (const auto *event = static_cast<const EventType *> (*cursor))
    {
        // We expect that there is no duplicates among events in these tests (for test code simplification).
        REQUIRE (std::find_if (found.begin (), found.end (),
                               [event] (const EventType *_found)
                               {
                                   return *event == *_found;
                               }) == found.end ());

        found.emplace_back (event);
        ++cursor;
    }

    CHECK_EQUAL (found.size (), expected.size ());
    for (const EventType *event : found)
    {
        CHECK (std::find_if (expected.begin (), expected.end (),
                             [event] (const EventType &_expected)
                             {
                                 return *event == _expected;
                             }) != expected.end ());
    }
}

template <typename EventType>
void AddValidatorTask (PipelineBuilder &_builder,
                       Container::Vector<EventType> _expected,
                       bool _insideExecutorPipeline = true)
{
    TaskConstructor constructor = _builder.AddTask (
        Memory::UniqueString {EMERGENCE_BUILD_STRING (EventType::Reflect ().mapping.GetName (), "Validator")});
    constructor.SetExecutor<Validator<EventType>> (std::move (_expected));

    if (_insideExecutorPipeline)
    {
        constructor.DependOn (EXECUTOR_TASK_NAME);
    }
}

} // namespace Tasks
} // namespace
} // namespace Emergence::Celerity::Test

using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test;
using namespace Emergence::Celerity::Test::Tasks;

BEGIN_SUITE (EventTriggering)

TEST_CASE (OnAdd)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        registrar.OnAddEvent ({{TestRecordAddedEvent::Reflect ().mapping, EventRoute::FIXED},
                               TestRecord::Reflect ().mapping,
                               {{TestRecord::Reflect ().id, TestRecordAddedEvent::Reflect ().id}}});
    }

    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, {AddRecord {{13u}}, AddRecord {{42u}}});
    AddValidatorTask<TestRecordAddedEvent> (builder, {{13u}, {42u}});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (pipeline);
    pipeline->Execute ();
}

TEST_CASE (OnAddShared)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        registrar.OnAddEvent ({{TestRecordAddedEvent::Reflect ().mapping, EventRoute::FIXED},
                               TestRecord::Reflect ().mapping,
                               {{TestRecord::Reflect ().id, TestRecordAddedEvent::Reflect ().id}}});

        registrar.OnAddEvent ({{TestRecordAddedSharedEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
                               TestRecord::Reflect ().mapping,
                               {
                                   {TestRecord::Reflect ().id, TestRecordAddedSharedEvent::Reflect ().id},
                                   {TestRecord::Reflect ().health, TestRecordAddedSharedEvent::Reflect ().health},
                               }});
    }

    const TestRecord testRecord {21u, 91.7f, 22.3f, 11.8f, 67.2f};
    PipelineBuilder builder {&world};

    builder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, {AddRecord {testRecord}});
    AddValidatorTask<TestRecordAddedEvent> (builder, {{testRecord.id}});
    Pipeline *fixedPipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (fixedPipeline);

    builder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    AddValidatorTask<TestRecordAddedSharedEvent> (builder, {{testRecord.id, testRecord.health}}, false);
    Pipeline *normalPipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (normalPipeline);

    fixedPipeline->Execute ();
    normalPipeline->Execute ();
}

TEST_CASE (OnRemove)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        registrar.OnRemoveEvent ({{TestRecordRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
                                  TestRecord::Reflect ().mapping,
                                  {{TestRecord::Reflect ().id, TestRecordRemovedEvent::Reflect ().id}}});
    }

    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, {AddRecord {{39u}}, AddRecord {{68u}}, RemoveRecord {39u}});
    AddValidatorTask<TestRecordRemovedEvent> (builder, {{39u}});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (pipeline);
    pipeline->Execute ();
}

TEST_CASE (OnRemoveMultiblockCopyout)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        registrar.OnRemoveEvent ({{TestRecordRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
                                  TestRecord::Reflect ().mapping,
                                  {
                                      {TestRecord::Reflect ().id, TestRecordRemovedEvent::Reflect ().id},
                                      {TestRecord::Reflect ().x, TestRecordRemovedEvent::Reflect ().x},
                                      {TestRecord::Reflect ().y, TestRecordRemovedEvent::Reflect ().y},
                                      {TestRecord::Reflect ().angle, TestRecordRemovedEvent::Reflect ().angle},
                                  }});
    }

    PipelineBuilder builder {&world};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);

    const TestRecord testRecord {42u, 97.3f, 22.3f, 11.8f, 67.2f};

    AddExecutorTask (builder, {AddRecord {testRecord}, RemoveRecord {testRecord.id}});
    AddValidatorTask<TestRecordRemovedEvent> (builder, {{testRecord.id, testRecord.x, testRecord.y, testRecord.angle}});

    Pipeline *pipeline = builder.End (std::thread::hardware_concurrency ());
    REQUIRE (pipeline);
    pipeline->Execute ();
}

END_SUITE
