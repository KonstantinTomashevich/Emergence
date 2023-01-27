#include <cstdint>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/World.hpp>

#include <Container/Variant.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Emergence::Memory::Literals;

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

    static const Reflection &Reflect () noexcept;
};

Container::String Emergence::Celerity::Test::TestRecord::ToString () const noexcept
{
    return EMERGENCE_BUILD_STRING ("{ id = ", id, ", health = ", health, ", x = ", x, ", y = ", y, ", angle = ", angle,
                                   " }");
}

const TestRecord::Reflection &TestRecord::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestRecord);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTER_REGULAR (angle);
        EMERGENCE_MAPPING_REGISTER_REGULAR (health);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

EMERGENCE_CELERITY_EVENT1_DECLARATION (TestRecordAddedEvent, uint64_t, id);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TestRecordAddedEvent, id)

void RegisterTestRecordAddedEvent (EventRegistrar &_registrar)
{
    _registrar.OnAddEvent ({{TestRecordAddedEvent::Reflect ().mapping, EventRoute::FIXED},
                            TestRecord::Reflect ().mapping,
                            {{TestRecord::Reflect ().id, TestRecordAddedEvent::Reflect ().id}}});
}

/// \details In lots of cases we need to know about addition in several pipelines (fixed and normal, for example).
///          In order to do it, we need to have two event types: one per pipeline. We're using this event type in pair
///          with TestRecordAddedEvent to check that this behaviour works correctly.
EMERGENCE_CELERITY_EVENT2_DECLARATION (TestRecordAddedSharedEvent, uint64_t, id, float, health);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (TestRecordAddedSharedEvent, id, health)

void RegisterTestRecordAddedSharedEvent (EventRegistrar &_registrar)
{
    _registrar.OnAddEvent ({{TestRecordAddedSharedEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
                            TestRecord::Reflect ().mapping,
                            {
                                {TestRecord::Reflect ().id, TestRecordAddedSharedEvent::Reflect ().id},
                                {TestRecord::Reflect ().health, TestRecordAddedSharedEvent::Reflect ().health},
                            }});
}

// Intentionally copy everything except health to test copy out of separate blocks.
EMERGENCE_CELERITY_EVENT4_DECLARATION (TestRecordRemovedEvent, uint64_t, id, float, x, float, y, float, angle);

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (TestRecordRemovedEvent, id, x, y, angle)

void RegisterTestRecordRemovedEvent (EventRegistrar &_registrar)
{
    _registrar.OnRemoveEvent ({{TestRecordRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
                               TestRecord::Reflect ().mapping,
                               {
                                   {TestRecord::Reflect ().id, TestRecordRemovedEvent::Reflect ().id},
                                   {TestRecord::Reflect ().x, TestRecordRemovedEvent::Reflect ().x},
                                   {TestRecord::Reflect ().y, TestRecordRemovedEvent::Reflect ().y},
                                   {TestRecord::Reflect ().angle, TestRecordRemovedEvent::Reflect ().angle},
                               }});
}

EMERGENCE_CELERITY_EVENT2_DECLARATION (TestRecordHealthChangedEvent, uint64_t, id, float, previousHealth);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (TestRecordHealthChangedEvent, id, previousHealth)

void RegisterTestRecordHealthChangedEvent (EventRegistrar &_registrar)
{
    _registrar.OnChangeEvent (
        {{TestRecordHealthChangedEvent::Reflect ().mapping, EventRoute::FIXED},
         TestRecord::Reflect ().mapping,
         {TestRecord::Reflect ().health},
         {{TestRecord::Reflect ().health, TestRecordHealthChangedEvent::Reflect ().previousHealth}},
         {{TestRecord::Reflect ().id, TestRecordHealthChangedEvent::Reflect ().id}}});
}

EMERGENCE_CELERITY_EVENT4_DECLARATION (
    TestRecordTransformChangedEvent, uint64_t, id, float, previousX, float, previousY, float, previousAngle);

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (TestRecordTransformChangedEvent, id, previousX, previousY, previousAngle)

void RegisterTestRecordTransformChangedEvent (EventRegistrar &_registrar)
{
    _registrar.OnChangeEvent (
        {{TestRecordTransformChangedEvent::Reflect ().mapping, EventRoute::FIXED},
         TestRecord::Reflect ().mapping,
         {TestRecord::Reflect ().x, TestRecord::Reflect ().y, TestRecord::Reflect ().angle},
         {
             {TestRecord::Reflect ().x, TestRecordTransformChangedEvent::Reflect ().previousX},
             {TestRecord::Reflect ().y, TestRecordTransformChangedEvent::Reflect ().previousY},
             {TestRecord::Reflect ().angle, TestRecordTransformChangedEvent::Reflect ().previousAngle},
         },
         {{TestRecord::Reflect ().id, TestRecordTransformChangedEvent::Reflect ().id}}});
}

/// \details Although this event has no practical sense, it is used
///          to check how change detection in separate blocks works.
EMERGENCE_CELERITY_EVENT3_DECLARATION (
    TestRecordHealthOrAngleChangedEvent, uint64_t, id, float, previousHealth, float, previousAngle);

EMERGENCE_CELERITY_EVENT3_IMPLEMENTATION (TestRecordHealthOrAngleChangedEvent, id, previousHealth, previousAngle)

void RegisterTestRecordHealthOrAngleChangedEvent (EventRegistrar &_registrar)
{
    _registrar.OnChangeEvent (
        {{TestRecordHealthOrAngleChangedEvent::Reflect ().mapping, EventRoute::FIXED},
         TestRecord::Reflect ().mapping,
         {TestRecord::Reflect ().health, TestRecord::Reflect ().angle},
         {
             {TestRecord::Reflect ().health, TestRecordHealthOrAngleChangedEvent::Reflect ().previousHealth},
             {TestRecord::Reflect ().angle, TestRecordHealthOrAngleChangedEvent::Reflect ().previousAngle},
         },
         {{TestRecord::Reflect ().id, TestRecordHealthOrAngleChangedEvent::Reflect ().id}}});
}

/// \details Although this event has no practical sense, it is used to check how change tracker zoning works.
EMERGENCE_CELERITY_EVENT3_DECLARATION (
    TestRecordHealthOrXChangedEvent, uint64_t, id, float, previousHealth, float, previousX);

EMERGENCE_CELERITY_EVENT3_IMPLEMENTATION (TestRecordHealthOrXChangedEvent, id, previousHealth, previousX)

void RegisterTestRecordHealthOrXChangedEvent (EventRegistrar &_registrar)
{
    _registrar.OnChangeEvent (
        {{TestRecordHealthOrXChangedEvent::Reflect ().mapping, EventRoute::FIXED},
         TestRecord::Reflect ().mapping,
         {TestRecord::Reflect ().health, TestRecord::Reflect ().x},
         {
             {TestRecord::Reflect ().health, TestRecordHealthOrXChangedEvent::Reflect ().previousHealth},
             {TestRecord::Reflect ().x, TestRecordHealthOrXChangedEvent::Reflect ().previousX},
         },
         {{TestRecord::Reflect ().id, TestRecordHealthOrXChangedEvent::Reflect ().id}}});
}

namespace Tasks
{
struct AddRecord final
{
    TestRecord record;
};

struct EditRecord final
{
    uint64_t recordId;
    TestRecord replaceWith;
};

struct RemoveRecord final
{
    uint64_t recordId;
};

using Task = Container::Variant<Tasks::AddRecord, Tasks::EditRecord, Tasks::RemoveRecord>;

class Executor final : public TaskExecutorBase<Executor>
{
public:
    Executor (TaskConstructor &_constructor, Container::Vector<Task> _tasks) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertRecord;
    ModifyValueQuery modifyRecordById;
    Container::Vector<Task> tasks;

    /// \details We need to verify that change trackers works both when cursor is incremented and when cursor is
    ///          destructed. Therefore we switch between increment and immediate destruction under the hood.
    static bool checkEditionCursorIncrement;
};

bool Executor::checkEditionCursorIncrement = false;

Executor::Executor (TaskConstructor &_constructor, Container::Vector<Task> _tasks) noexcept
    : insertRecord (INSERT_LONG_TERM (TestRecord)),
      modifyRecordById (MODIFY_VALUE_1F (TestRecord, id)),
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
                else if constexpr (std::is_same_v<Tasks::EditRecord, Type>)
                {
                    LOG ("Editing record with id ", _task.recordId, " by replacing its content with ",
                         _task.replaceWith.ToString (), ".");
                    auto cursor = modifyRecordById.Execute (&_task.recordId);

                    if (auto *record = static_cast<TestRecord *> (*cursor))
                    {
                        *record = _task.replaceWith;
                        if (checkEditionCursorIncrement)
                        {
                            ++cursor;
                            CHECK_WITH_MESSAGE (!*cursor, "Record ids are expected to be unique.");
                        }

                        checkEditionCursorIncrement = !checkEditionCursorIncrement;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find record with id ", _task.recordId, ".");
                    }
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
    : fetch (FETCH_SEQUENCE (EventType)),
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

struct ChangeInfo final
{
    TestRecord initial;
    TestRecord changed;
};

using ChangeVector = Container::Vector<ChangeInfo>;

Container::Vector<Task> FromChangeVector (const ChangeVector &_changes)
{
    Container::Vector<Task> result;
    for (const ChangeInfo &change : _changes)
    {
        result.emplace_back (Tasks::AddRecord {change.initial});
        result.emplace_back (Tasks::EditRecord {change.initial.id, change.changed});
    }

    return result;
}

/// \brief Encapsulates simple routine, shared by most tests that interact with one event type.
template <typename EventType>
void SeparatedEventTest (World *_world, Container::Vector<Task> _tasks, Container::Vector<EventType> _expected)
{
    PipelineBuilder builder {_world->GetRootView ()};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, std::move (_tasks));
    AddValidatorTask<EventType> (builder, std::move (_expected));

    Pipeline *pipeline = builder.End ();
    REQUIRE (pipeline);
    pipeline->Execute ();
}

/// \brief Encapsulates logic for testing multiple on change event triggers.
/// \details We test several on change events registration orders to make sure that tracker zoning algorithm is correct.
void OnChangeMultipleTest (World *_world)
{
    const ChangeVector changes {
        {
            {11u, 98.4f, 23.1f, 43.6f, 67.9f},
            {11u, 67.f, 23.1f, 43.6f, 67.9f},
        },
        {
            {12u, 98.4f, 23.1f, 43.6f, 67.9f},
            {12u, 98.4f, 99.6f, 43.6f, 67.9f},
        },
        {
            {13u, 98.4f, 23.1f, 43.6f, 67.9f},
            {13u, 98.4f, 23.1f, 21.5f, 67.9f},
        },
        {
            {14u, 98.4f, 23.1f, 43.6f, 67.9f},
            {14u, 98.4f, 23.1f, 43.6f, 21.3f},
        },
        {
            {15u, 98.4f, 23.1f, 43.6f, 67.9f},
            {15u, 22.4f, 97.1f, 43.6f, 67.9f},
        },
        {
            {16u, 98.4f, 23.1f, 43.6f, 67.9f},
            {16u, 98.4f, 23.1f, 11.2f, 89.7f},
        },
        {
            {42u, 98.4f, 23.1f, 43.6f, 67.9f},
            {43u, 98.4f, 23.1f, 43.6f, 67.9f},
        },
    };

    PipelineBuilder builder {_world->GetRootView ()};
    builder.Begin ("Update"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, FromChangeVector (changes));

    AddValidatorTask<TestRecordHealthChangedEvent> (builder, {
                                                                 {changes[0u].initial.id, changes[0u].initial.health},
                                                                 {changes[4u].initial.id, changes[4u].initial.health},
                                                             });

    AddValidatorTask<TestRecordTransformChangedEvent> (
        builder, {
                     {changes[1u].initial.id, changes[1u].initial.x, changes[1u].initial.y, changes[1u].initial.angle},
                     {changes[2u].initial.id, changes[2u].initial.x, changes[2u].initial.y, changes[2u].initial.angle},
                     {changes[3u].initial.id, changes[3u].initial.x, changes[3u].initial.y, changes[3u].initial.angle},
                     {changes[4u].initial.id, changes[4u].initial.x, changes[4u].initial.y, changes[4u].initial.angle},
                     {changes[5u].initial.id, changes[5u].initial.x, changes[5u].initial.y, changes[5u].initial.angle},
                 });

    AddValidatorTask<TestRecordHealthOrAngleChangedEvent> (
        builder, {
                     {changes[0u].initial.id, changes[0u].initial.health, changes[0u].initial.angle},
                     {changes[3u].initial.id, changes[3u].initial.health, changes[3u].initial.angle},
                     {changes[4u].initial.id, changes[4u].initial.health, changes[4u].initial.angle},
                     {changes[5u].initial.id, changes[5u].initial.health, changes[5u].initial.angle},
                 });

    AddValidatorTask<TestRecordHealthOrXChangedEvent> (
        builder, {
                     {changes[0u].initial.id, changes[0u].initial.health, changes[0u].initial.x},
                     {changes[1u].initial.id, changes[1u].initial.health, changes[1u].initial.x},
                     {changes[4u].initial.id, changes[4u].initial.health, changes[4u].initial.x},
                 });

    Pipeline *pipeline = builder.End ();
    REQUIRE (pipeline);
    pipeline->Execute ();
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
        RegisterTestRecordAddedEvent (registrar);
    }

    SeparatedEventTest<TestRecordAddedEvent> (&world, {AddRecord {{13u}}, AddRecord {{42u}}}, {{13u}, {42u}});
}

TEST_CASE (OnAddShared)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordAddedEvent (registrar);
        RegisterTestRecordAddedSharedEvent (registrar);
    }

    const TestRecord testRecord {21u, 91.7f, 22.3f, 11.8f, 67.2f};
    PipelineBuilder builder {world.GetRootView ()};

    builder.Begin ("FixedUpdate"_us, Emergence::Celerity::PipelineType::FIXED);
    AddExecutorTask (builder, {AddRecord {testRecord}});
    AddValidatorTask<TestRecordAddedEvent> (builder, {{testRecord.id}});
    Pipeline *fixedPipeline = builder.End ();
    REQUIRE (fixedPipeline);

    builder.Begin ("NormalUpdate"_us, Emergence::Celerity::PipelineType::NORMAL);
    AddValidatorTask<TestRecordAddedSharedEvent> (builder, {{testRecord.id, testRecord.health}}, false);
    Pipeline *normalPipeline = builder.End ();
    REQUIRE (normalPipeline);

    fixedPipeline->Execute ();
    normalPipeline->Execute ();
}

TEST_CASE (OnRemove)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordRemovedEvent (registrar);
    }

    const TestRecord testRecord {42u, 97.3f, 22.3f, 11.8f, 67.2f};
    SeparatedEventTest<TestRecordRemovedEvent> (&world, {AddRecord {testRecord}, RemoveRecord {testRecord.id}},
                                                {{testRecord.id, testRecord.x, testRecord.y, testRecord.angle}});
}

TEST_CASE (OnChangeTrivial)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordHealthChangedEvent (registrar);
    }

    const ChangeVector changes {
        {
            {42u, 97.3f, 22.3f, 11.8f, 67.2f},
            {42u, 65.7f, 29.3f, 17.8f, 67.2f},
        },
        {
            {43u, 97.3f, 22.3f, 11.8f, 67.2f},
            {43u, 97.3f, 22.9f, 11.1f, 67.7f},
        },
    };

    SeparatedEventTest<TestRecordHealthChangedEvent> (&world, FromChangeVector (changes),
                                                      {{changes[0u].initial.id, changes[0u].initial.health}});
}

TEST_CASE (OnChangeContiniousFieldBlock)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordTransformChangedEvent (registrar);
    }

    const ChangeVector changes {
        {
            {42u, 97.3f, 22.3f, 11.8f, 67.2f},
            {42u, 97.3f, 22.8f, 11.8f, 67.2f},
        },
        {
            {43u, 97.3f, 22.3f, 11.8f, 67.2f},
            {43u, 97.3f, 22.3f, 11.3f, 67.2f},
        },
        {
            {44u, 97.3f, 22.3f, 11.8f, 67.2f},
            {44u, 97.3f, 22.3f, 11.8f, 67.8f},
        },
        {
            {45u, 97.3f, 22.3f, 11.8f, 67.2f},
            {45u, 61.4f, 22.3f, 11.8f, 67.2f},
        },
    };

    SeparatedEventTest<TestRecordTransformChangedEvent> (
        &world, FromChangeVector (changes),
        {
            {changes[0u].initial.id, changes[0u].initial.x, changes[0u].initial.y, changes[0u].initial.angle},
            {changes[1u].initial.id, changes[1u].initial.x, changes[1u].initial.y, changes[1u].initial.angle},
            {changes[2u].initial.id, changes[2u].initial.x, changes[2u].initial.y, changes[2u].initial.angle},
        });
}

TEST_CASE (OnChangeSeparatedFieldBlocks)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordHealthOrAngleChangedEvent (registrar);
    }

    const ChangeVector changes {
        {
            {42u, 97.3f, 22.3f, 11.8f, 67.2f},
            {42u, 65.3f, 22.3f, 11.8f, 67.2f},
        },
        {
            {43u, 97.3f, 22.3f, 11.8f, 67.2f},
            {43u, 97.3f, 43.1f, 11.3f, 67.2f},
        },
        {
            {44u, 97.3f, 22.3f, 11.8f, 67.2f},
            {44u, 97.3f, 22.9f, 11.8f, 67.8f},
        },
    };

    SeparatedEventTest<TestRecordHealthOrAngleChangedEvent> (
        &world, FromChangeVector (changes),
        {
            {changes[0u].initial.id, changes[0u].initial.health, changes[0u].initial.angle},
            {changes[2u].initial.id, changes[2u].initial.health, changes[2u].initial.angle},
        });
}

TEST_CASE (OnChangeMultipleFirstOrder)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordTransformChangedEvent (registrar);
        RegisterTestRecordHealthOrAngleChangedEvent (registrar);
        RegisterTestRecordHealthChangedEvent (registrar);
        RegisterTestRecordHealthOrXChangedEvent (registrar);
    }

    OnChangeMultipleTest (&world);
}

TEST_CASE (OnChangeMultipleSecondOrder)
{
    World world {"TestWorld"_us};

    {
        EventRegistrar registrar {&world};
        RegisterTestRecordTransformChangedEvent (registrar);
        RegisterTestRecordHealthOrXChangedEvent (registrar);
        RegisterTestRecordHealthChangedEvent (registrar);
        RegisterTestRecordHealthOrAngleChangedEvent (registrar);
    }

    OnChangeMultipleTest (&world);
}

END_SUITE
