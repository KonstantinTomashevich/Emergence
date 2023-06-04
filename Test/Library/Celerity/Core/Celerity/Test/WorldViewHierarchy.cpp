#include <cstdint>

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Standard/UniqueId.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using namespace Emergence::Memory::Literals;

struct TestComponent final
{
    UniqueId objectId = INVALID_UNIQUE_ID;
    float x = 0.0f;
    float y = 0.0f;

    bool operator== (const TestComponent &_other) const noexcept = default;

    bool operator!= (const TestComponent &_other) const noexcept = default;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TestComponent::Reflection &TestComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

EMERGENCE_CELERITY_EVENT1_DECLARATION (TestComponentAddedNormalEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (TestComponentChangedNormalEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (TestComponentRemovedNormalEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TestComponentAddedNormalEvent, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TestComponentChangedNormalEvent, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TestComponentRemovedNormalEvent, objectId)

struct EditionFrame final
{
    Container::Vector<TestComponent> toAdd;
    Container::Vector<TestComponent> toEdit;
    Container::Vector<UniqueId> toRemove;
};

static const Memory::UniqueString EDITION_FINISHED_CHECKPOINT {"EditionFinished"};

class Editor final : public TaskExecutorBase<Editor>
{
public:
    Editor (TaskConstructor &_constructor, Container::Vector<EditionFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    InsertLongTermQuery insertComponent;
    ModifyValueQuery modifyComponent;

    std::uint32_t frameIndex = 0u;
    Container::Vector<EditionFrame> frames;
};

Editor::Editor (TaskConstructor &_constructor, Container::Vector<EditionFrame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      insertComponent (INSERT_LONG_TERM (TestComponent)),
      modifyComponent (MODIFY_VALUE_1F (TestComponent, objectId)),
      frames (std::move (_frames))
{
    _constructor.MakeDependencyOf (EDITION_FINISHED_CHECKPOINT);
}

void Editor::Execute () noexcept
{
    if (frameIndex >= frames.size ())
    {
        return;
    }

    const EditionFrame &frame = frames[frameIndex];
    ++frameIndex;

    {
        auto cursor = insertComponent.Execute ();
        for (const TestComponent &component : frame.toAdd)
        {
            *static_cast<TestComponent *> (++cursor) = component;
        }
    }

    for (const TestComponent &component : frame.toEdit)
    {
        auto cursor = modifyComponent.Execute (&component.objectId);
        REQUIRE (*cursor);
        *static_cast<TestComponent *> (*cursor) = component;
    }

    for (UniqueId objectId : frame.toRemove)
    {
        auto cursor = modifyComponent.Execute (&objectId);
        REQUIRE (*cursor);
        ~cursor;
    }
}

using ComponentVerificationFrame = Container::Vector<TestComponent>;

class ComponentVerifier final : public TaskExecutorBase<ComponentVerifier>
{
public:
    ComponentVerifier (TaskConstructor &_constructor, Container::Vector<ComponentVerificationFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    FetchAscendingRangeQuery fetchComponents;
    std::size_t frameIndex = 0u;
    Container::Vector<ComponentVerificationFrame> frames;
};

ComponentVerifier::ComponentVerifier (TaskConstructor &_constructor,
                                      Container::Vector<ComponentVerificationFrame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      fetchComponents (FETCH_ASCENDING_RANGE (TestComponent, objectId)),
      frames (std::move (_frames))
{
    _constructor.DependOn (EDITION_FINISHED_CHECKPOINT);
}

void ComponentVerifier::Execute () noexcept
{
    if (frameIndex >= frames.size ())
    {
        return;
    }

    const ComponentVerificationFrame &frame = frames[frameIndex];
    ++frameIndex;

    std::size_t componentIndex = 0u;
    for (auto cursor = fetchComponents.Execute (nullptr, nullptr);
         const auto *component = static_cast<const TestComponent *> (*cursor); ++cursor, ++componentIndex)
    {
        if (componentIndex < frame.size ())
        {
            CHECK_EQUAL (*component, frame[componentIndex]);
        }
    }

    CHECK_EQUAL (componentIndex, frame.size ());
}

struct EventVerificationFrame final
{
    Container::Vector<TestComponentAddedNormalEvent> expectedAddedEvents;
    Container::Vector<TestComponentChangedNormalEvent> expectedChangedEvents;
    Container::Vector<TestComponentRemovedNormalEvent> expectedRemovedEvents;
};

class EventVerifier final : public TaskExecutorBase<EventVerifier>
{
public:
    EventVerifier (TaskConstructor &_constructor, Container::Vector<EventVerificationFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchAddedEvents;
    FetchSequenceQuery fetchChangedEvents;
    FetchSequenceQuery fetchRemovedEvents;

    std::size_t frameIndex = 0u;
    Container::Vector<EventVerificationFrame> frames;
};

EventVerifier::EventVerifier (TaskConstructor &_constructor, Container::Vector<EventVerificationFrame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      fetchAddedEvents (FETCH_SEQUENCE (TestComponentAddedNormalEvent)),
      fetchChangedEvents (FETCH_SEQUENCE (TestComponentChangedNormalEvent)),
      fetchRemovedEvents (FETCH_SEQUENCE (TestComponentRemovedNormalEvent)),
      frames (std::move (_frames))
{
    _constructor.DependOn (EDITION_FINISHED_CHECKPOINT);
}

void EventVerifier::Execute () noexcept
{
    if (frameIndex >= frames.size ())
    {
        return;
    }

    const EventVerificationFrame &frame = frames[frameIndex];
    ++frameIndex;

#define CHECK_EVENTS(Type)                                                                                             \
    {                                                                                                                  \
        Container::Vector<TestComponent##Type##NormalEvent> eventsFound;                                               \
        for (auto cursor = fetch##Type##Events.Execute ();                                                             \
             const auto *event = static_cast<const TestComponent##Type##NormalEvent *> (*cursor); ++cursor)            \
        {                                                                                                              \
            eventsFound.emplace_back (*event);                                                                         \
        }                                                                                                              \
                                                                                                                       \
        CHECK_EQUAL (eventsFound.size (), frame.expected##Type##Events.size ());                                       \
        for (const TestComponent##Type##NormalEvent &event : eventsFound)                                              \
        {                                                                                                              \
            const std::size_t found = std::count (eventsFound.begin (), eventsFound.end (), event);                    \
            const std::size_t expected =                                                                               \
                std::count (frame.expected##Type##Events.begin (), frame.expected##Type##Events.end (), event);        \
            CHECK_EQUAL (found, expected);                                                                             \
        }                                                                                                              \
    }

    CHECK_EVENTS (Added)
    CHECK_EVENTS (Changed)
    CHECK_EVENTS (Removed)
}

void RegisterTestEvents (World &_world)
{
    EventRegistrar registrar {&_world};
    registrar.OnAddEvent ({{TestComponentAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                           TestComponent::Reflect ().mapping,
                           {{TestComponent::Reflect ().objectId, TestComponentAddedNormalEvent::Reflect ().objectId}}});

    registrar.OnChangeEvent (
        {{TestComponentChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         TestComponent::Reflect ().mapping,
         {TestComponent::Reflect ().x, TestComponent::Reflect ().y},
         {},
         {{TestComponent::Reflect ().objectId, TestComponentChangedNormalEvent::Reflect ().objectId}}});

    registrar.OnRemoveEvent (
        {{TestComponentRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         TestComponent::Reflect ().mapping,
         {{TestComponent::Reflect ().objectId, TestComponentRemovedNormalEvent::Reflect ().objectId}}});
}

void TestObjectAndEventPropagationToRoot (bool _explicit)
{
    WorldConfiguration configuration;
    if (_explicit)
    {
        configuration.rootViewConfig.enforcedTypes.emplace (TestComponent::Reflect ().mapping);
        configuration.rootViewConfig.enforcedTypes.emplace (TestComponentAddedNormalEvent::Reflect ().mapping);
        configuration.rootViewConfig.enforcedTypes.emplace (TestComponentChangedNormalEvent::Reflect ().mapping);
        configuration.rootViewConfig.enforcedTypes.emplace (TestComponentRemovedNormalEvent::Reflect ().mapping);
    }

    World world {"TestWorld"_us, configuration};
    RegisterTestEvents (world);

    WorldView *firstView = world.CreateView (world.GetRootView (), "First"_us, {});
    WorldView *secondView = world.CreateView (world.GetRootView (), "Second"_us, {});

    const ComponentVerificationFrame allComponents {
        TestComponent {0u, 1.5f, 1.5f},
        TestComponent {1u, 2.5f, 3.5f},
        TestComponent {2u, 4.9f, 0.7f},
    };

    if (!_explicit)
    {
        PipelineBuilder rootBuilder {world.GetRootView ()};
        rootBuilder.Begin ("Update"_us, PipelineType::NORMAL);
        rootBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

        rootBuilder.AddTask ("ComponentVerifier"_us)
            .SetExecutor<ComponentVerifier> (Container::Vector<ComponentVerificationFrame> {{}, allComponents});

        rootBuilder.AddTask ("EventVerifier"_us)
            .SetExecutor<EventVerifier> (
                Container::Vector<EventVerificationFrame> {{},
                                                           {
                                                               {
                                                                   TestComponentAddedNormalEvent {0u},
                                                                   TestComponentAddedNormalEvent {1u},
                                                                   TestComponentAddedNormalEvent {2u},
                                                               },
                                                               {},
                                                               {},
                                                           }});
        REQUIRE (rootBuilder.End ());
    }

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {0u, 1.5f, 1.5f},
            },
            {},
            {},
        }});

    firstViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           },
                                                           allComponents});
    REQUIRE (firstViewBuilder.End ());

    PipelineBuilder secondViewBuilder {secondView};
    secondViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    secondViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    secondViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {1u, 2.5f, 3.5f},
                TestComponent {2u, 4.9f, 0.7f},
            },
            {},
            {},
        }});

    secondViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (Container::Vector<ComponentVerificationFrame> {allComponents, allComponents});
    REQUIRE (secondViewBuilder.End ());

    world.Update ();
    world.Update ();
}
} // namespace Emergence::Celerity::Test

using namespace Emergence;
using namespace Emergence::Celerity;
using namespace Emergence::Celerity::Test;

BEGIN_SUITE (WorldViewHierarchy)

TEST_CASE (ImplicitPropagation)
{
    TestObjectAndEventPropagationToRoot (false);
}

TEST_CASE (ExplicitPropagation)
{
    TestObjectAndEventPropagationToRoot (true);
}

TEST_CASE (NoPropagation)
{
    World world {"TestWorld"_us};
    RegisterTestEvents (world);

    WorldView *firstView = world.CreateView (world.GetRootView (), "First"_us, {});
    WorldView *secondView = world.CreateView (world.GetRootView (), "Second"_us, {});

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {0u, 1.5f, 1.5f},
            },
            {},
            {},
        }});

    firstViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           },
                                                           {
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           }});
    REQUIRE (firstViewBuilder.End ());

    PipelineBuilder secondViewBuilder {secondView};
    secondViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    secondViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    secondViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {1u, 2.5f, 3.5f},
                TestComponent {2u, 4.9f, 0.7f},
            },
            {},
            {},
        }});

    secondViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {1u, 2.5f, 3.5f},
                                                               TestComponent {2u, 4.9f, 0.7f},
                                                           },
                                                           {
                                                               TestComponent {1u, 2.5f, 3.5f},
                                                               TestComponent {2u, 4.9f, 0.7f},
                                                           }});
    REQUIRE (secondViewBuilder.End ());

    world.Update ();
    world.Update ();
}

TEST_CASE (EventOnlyPropagation)
{
    World world {"TestWorld"_us};
    RegisterTestEvents (world);

    WorldView *firstView = world.CreateView (world.GetRootView (), "First"_us, {});
    WorldView *secondView = world.CreateView (world.GetRootView (), "Second"_us, {});

    PipelineBuilder rootBuilder {world.GetRootView ()};
    rootBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    rootBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    rootBuilder.AddTask ("EventVerifier"_us)
        .SetExecutor<EventVerifier> (
            Container::Vector<EventVerificationFrame> {{},
                                                       {
                                                           {
                                                               TestComponentAddedNormalEvent {0u},
                                                               TestComponentAddedNormalEvent {1u},
                                                               TestComponentAddedNormalEvent {2u},
                                                           },
                                                           {},
                                                           {},
                                                       }});
    REQUIRE (rootBuilder.End ());

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {0u, 1.5f, 1.5f},
            },
            {},
            {},
        }});

    firstViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           },
                                                           {
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           }});
    REQUIRE (firstViewBuilder.End ());

    PipelineBuilder secondViewBuilder {secondView};
    secondViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    secondViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    secondViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {1u, 2.5f, 3.5f},
                TestComponent {2u, 4.9f, 0.7f},
            },
            {},
            {},
        }});

    secondViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {1u, 2.5f, 3.5f},
                                                               TestComponent {2u, 4.9f, 0.7f},
                                                           },
                                                           {
                                                               TestComponent {1u, 2.5f, 3.5f},
                                                               TestComponent {2u, 4.9f, 0.7f},
                                                           }});
    REQUIRE (secondViewBuilder.End ());

    world.Update ();
    world.Update ();
}

TEST_CASE (RemovalEventsOnDrop)
{
    World world {"TestWorld"_us};
    RegisterTestEvents (world);
    WorldView *firstView = world.CreateView (world.GetRootView (), "First"_us, {});

    PipelineBuilder rootBuilder {world.GetRootView ()};
    rootBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    rootBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    rootBuilder.AddTask ("EventVerifier"_us)
        .SetExecutor<EventVerifier> (
            Container::Vector<EventVerificationFrame> {{},
                                                       {
                                                           {
                                                               TestComponentAddedNormalEvent {0u},
                                                               TestComponentAddedNormalEvent {1u},
                                                           },
                                                           {},
                                                           {},
                                                       },
                                                       {
                                                           {

                                                           },
                                                           {},
                                                           {
                                                               TestComponentRemovedNormalEvent {0u},
                                                               TestComponentRemovedNormalEvent {1u},
                                                           },
                                                       }});
    REQUIRE (rootBuilder.End ());

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {0u, 1.5f, 1.5f},
                TestComponent {1u, 2.5f, 3.5f},
            },
            {},
            {},
        }});

    REQUIRE (firstViewBuilder.End ());

    world.Update ();
    world.Update ();
    world.DropView (firstView);
    world.Update ();
}

TEST_CASE (MultiLevel)
{
    WorldConfiguration configuration;
    configuration.rootViewConfig.enforcedTypes.emplace (TestComponent::Reflect ().mapping);
    configuration.rootViewConfig.enforcedTypes.emplace (TestComponentAddedNormalEvent::Reflect ().mapping);
    configuration.rootViewConfig.enforcedTypes.emplace (TestComponentChangedNormalEvent::Reflect ().mapping);
    configuration.rootViewConfig.enforcedTypes.emplace (TestComponentRemovedNormalEvent::Reflect ().mapping);

    World world {"TestWorld"_us, configuration};
    RegisterTestEvents (world);

    WorldView *firstIntermediateView = world.CreateView (world.GetRootView (), "FirstIntermediate"_us, {});
    WorldView *firstView = world.CreateView (firstIntermediateView, "First"_us, {});
    WorldView *secondIntermediateView = world.CreateView (world.GetRootView (), "SecondIntermediate"_us, {});
    WorldView *secondView = world.CreateView (secondIntermediateView, "Second"_us, {});

    PipelineBuilder rootBuilder {world.GetRootView ()};
    rootBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    rootBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    const ComponentVerificationFrame allComponents {
        TestComponent {0u, 1.5f, 1.5f},
        TestComponent {1u, 2.5f, 3.5f},
        TestComponent {2u, 4.9f, 0.7f},
    };

    rootBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (Container::Vector<ComponentVerificationFrame> {{}, allComponents});

    REQUIRE (rootBuilder.End ());

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {0u, 1.5f, 1.5f},
            },
            {},
            {},
        }});

    firstViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (
            Container::Vector<ComponentVerificationFrame> {{
                                                               TestComponent {0u, 1.5f, 1.5f},
                                                           },
                                                           allComponents});
    REQUIRE (firstViewBuilder.End ());

    PipelineBuilder secondViewBuilder {secondView};
    secondViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    secondViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    secondViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {{
            {
                TestComponent {1u, 2.5f, 3.5f},
                TestComponent {2u, 4.9f, 0.7f},
            },
            {},
            {},
        }});

    secondViewBuilder.AddTask ("ComponentVerifier"_us)
        .SetExecutor<ComponentVerifier> (Container::Vector<ComponentVerificationFrame> {allComponents, allComponents});
    REQUIRE (secondViewBuilder.End ());

    world.Update ();
    world.Update ();
}

TEST_CASE (AllEventTypes)
{
    World world {"TestWorld"_us};
    RegisterTestEvents (world);
    WorldView *firstView = world.CreateView (world.GetRootView (), "First"_us, {});

    PipelineBuilder rootBuilder {world.GetRootView ()};
    rootBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    rootBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    rootBuilder.AddTask ("EventVerifier"_us)
        .SetExecutor<EventVerifier> (Container::Vector<EventVerificationFrame> {
            {},
            {
                {
                    TestComponentAddedNormalEvent {0u},
                },
                {},
                {},
            },
            {
                {},
                {
                    TestComponentChangedNormalEvent {0u},
                },
                {},
            },
            {
                {},
                {},
                {
                    TestComponentRemovedNormalEvent {0u},
                },
            },
        });
    REQUIRE (rootBuilder.End ());

    PipelineBuilder firstViewBuilder {firstView};
    firstViewBuilder.Begin ("Update"_us, PipelineType::NORMAL);
    firstViewBuilder.AddCheckpoint (EDITION_FINISHED_CHECKPOINT);

    firstViewBuilder.AddTask ("Editor"_us)
        .SetExecutor<Editor> (Container::Vector<EditionFrame> {
            {
                {
                    TestComponent {0u, 1.5f, 1.5f},
                },
                {},
                {},
            },
            {
                {},
                {
                    TestComponent {0u, 3.5f, 9.5f},
                },
                {},
            },
            {
                {},
                {},
                {0u},
            },
        });

    REQUIRE (firstViewBuilder.End ());
    world.Update ();
    world.Update ();
    world.Update ();
    world.Update ();
}

END_SUITE
