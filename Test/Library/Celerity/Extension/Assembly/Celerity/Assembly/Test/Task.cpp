#define _CRT_SECURE_NO_WARNINGS

#include <SyntaxSugar/MuteWarnings.hpp>

BEGIN_MUTING_PADDING_WARNING

#include <cstring>

#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Assembly/Test/Task.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/PatchBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorTask> _tasks) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchWorld;

    InsertLongTermQuery insertAssemblyDescriptor;
    InsertLongTermQuery insertTransform;
    InsertLongTermQuery insertPrototype;

    Container::Vector<ConfiguratorTask> tasks;
};

Configurator::Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorTask> _tasks) noexcept
    : fetchWorld (FETCH_SINGLETON (WorldSingleton)),

      insertAssemblyDescriptor (INSERT_LONG_TERM (AssemblyDescriptor)),
      insertTransform (INSERT_LONG_TERM (Transform3dComponent)),
      insertPrototype (INSERT_LONG_TERM (PrototypeComponent)),

      tasks (std::move (_tasks))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Assembly::Checkpoint::STARTED);
}

void Configurator::Execute () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const WorldSingleton *> (*worldCursor);

    static_assert (ASSEMBLY_ROOT_OBJECT_ID == 0u);
    // We're skipping first zero id in order to be sure that id replacement works correctly.
    world->GenerateId ();

    for (const ConfiguratorTask &task : tasks)
    {
        std::visit (
            [this, world] (const auto &_task)
            {
                using namespace Tasks;
                using Task = std::decay_t<decltype (_task)>;

                if constexpr (std::is_same_v<Task, AddAssemblyDescriptor>)
                {
                    LOG ("Adding assembly descriptor \"", *_task.descriptorId, "\".");
                    auto cursor = insertAssemblyDescriptor.Execute ();
                    auto *descriptor = static_cast<AssemblyDescriptor *> (++cursor);
                    descriptor->id = _task.descriptorId;

                    for (const PatchSource &source : _task.patches)
                    {
                        static std::array<std::uint8_t, 1024u> buffer;
                        std::uint8_t *data = buffer.data ();

                        // Make sure that alignment requirements are met.
                        while (reinterpret_cast<uintptr_t> (data) % source.type.GetObjectAlignment () != 0u)
                        {
                            ++data;
                        }

                        REQUIRE (static_cast<std::size_t> (&*buffer.end () - data) >= source.type.GetObjectSize ());
                        source.type.Construct (data);
                        descriptor->components.emplace_back (StandardLayout::PatchBuilder::FromDifference (
                            source.type, source.changedAfterCreation, data));
                        source.type.Destruct (data);
                    }
                }
                else if constexpr (std::is_same_v<Task, SpawnPrototype>)
                {
                    LOG ("Spawning prototype from descriptor \"", *_task.descriptorId, "\".");
                    const UniqueId objectId = world->GenerateId ();

                    auto transformCursor = insertTransform.Execute ();
                    auto *transform = static_cast<Transform3dComponent *> (++transformCursor);
                    transform->SetObjectId (objectId);

                    if (_task.logicalTransform)
                    {
                        transform->SetLogicalLocalTransform (_task.transform);
                    }
                    else
                    {
                        transform->SetVisualLocalTransform (_task.transform);
                    }

                    auto prototypeCursor = insertPrototype.Execute ();
                    auto *prototype = static_cast<PrototypeComponent *> (++prototypeCursor);
                    prototype->objectId = objectId;
                    prototype->descriptorId = _task.descriptorId;
                }
            },
            task);
    }
}

class Validator final : public TaskExecutorBase<Validator>
{
public:
    Validator (TaskConstructor &_constructor, Container::Vector<ValidatorTask> _tasks) noexcept;

    void Execute () noexcept;

private:
    Container::Vector<ValidatorTask> tasks;
    Container::Vector<Container::Optional<FetchValueQuery>> associatedQueries;
};

Validator::Validator (TaskConstructor &_constructor, Container::Vector<ValidatorTask> _tasks) noexcept
    : tasks (std::move (_tasks))
{
    _constructor.DependOn (Assembly::Checkpoint::FINISHED);

    for (const ValidatorTask &task : tasks)
    {
        Container::Optional<FetchValueQuery> &associatedQuery = associatedQueries.emplace_back (std::nullopt);
        std::visit (
            [&associatedQuery, &_constructor] (const auto &_task)
            {
                using namespace Tasks;
                using Task = std::decay_t<decltype (_task)>;

                if constexpr (std::is_same_v<Task, CheckComponent> || std::is_same_v<Task, CheckVector3f>)
                {
                    associatedQuery.emplace (_constructor.FetchValue (_task.type, {_task.idField}));
                }
            },
            task);
    }
}

void Validator::Execute () noexcept
{
    for (std::size_t index = 0u; index < tasks.size (); ++index)
    {
        std::visit (
            [this, index] (const auto &_task)
            {
                using namespace Tasks;
                using Task = std::decay_t<decltype (_task)>;

                if constexpr (std::is_same_v<Task, CheckComponent>)
                {
                    LOG ("Checking component \"", *_task.type.GetName (), "\" with id ", _task.id, " (in field \"",
                         *_task.type.GetField (_task.idField).GetName (), "\").");
                    auto cursor = associatedQueries[index].value ().Execute (&_task.id);

                    if (_task.expected)
                    {
                        if (*cursor)
                        {
                            const bool equal =
                                memcmp (*cursor, _task.expected,
                                        _task.comparisonSliceLength == 0u ? _task.type.GetObjectSize () :
                                                                            _task.comparisonSliceLength) == 0;

                            CHECK_WITH_MESSAGE (
                                equal, "Expected and pointed components should be equal!\nComponent: ",
                                EMERGENCE_BUILD_STRING (Container::StringBuilder::ObjectPointer {*cursor, _task.type}),
                                "\nExpected components: ",
                                EMERGENCE_BUILD_STRING (
                                    Container::StringBuilder::ObjectPointer {_task.expected, _task.type}));
                        }
                        else
                        {
                            CHECK_WITH_MESSAGE (false, "Unable to find component with id ", _task.id, "!");
                        }
                    }
                    else
                    {
                        CHECK_EQUAL (*cursor, nullptr);
                    }
                }
                else if constexpr (std::is_same_v<Task, CheckVector3f>)
                {
                    LOG ("Checking vector \"", *_task.type.GetField (_task.vectorField).GetName (),
                         "\" of component \"", *_task.type.GetName (), "\" with id ", _task.id, " (in field \"",
                         *_task.type.GetField (_task.idField).GetName (), "\").");
                    auto cursor = associatedQueries[index].value ().Execute (&_task.id);

                    if (*cursor)
                    {
                        StandardLayout::Field field = _task.type.GetField (_task.vectorField);
                        const auto *value = static_cast<const Math::Vector3f *> (field.GetValue (*cursor));
                        CHECK (Math::NearlyEqual (*value, _task.expected));
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find component with id ", _task.id, "!");
                    }
                }
            },
            tasks[index]);
    }
}

using namespace Memory::Literals;

void AddConfiguratorAndValidator (PipelineBuilder &_pipelineBuilder,
                                  Container::Vector<ConfiguratorTask> _configuratorTasks,
                                  Container::Vector<ValidatorTask> _validatorTasks)
{
    _pipelineBuilder.AddTask ("Configurator"_us).SetExecutor<Configurator> (std::move (_configuratorTasks));
    _pipelineBuilder.AddTask ("Validator"_us).SetExecutor<Validator> (std::move (_validatorTasks));
}
} // namespace Emergence::Celerity::Test

END_MUTING_WARNINGS
