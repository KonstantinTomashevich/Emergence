#include <optional>
#include <sstream>
#include <unordered_map>

#include <Pegasus/Storage.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/CursorManager.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Pegasus::Test
{
using IndexReference = std::variant <
    Handling::Handle <HashIndex>,
    Handling::Handle <OrderedIndex>,
    Handling::Handle <VolumetricIndex>>;

class ExecutionContext final
{
public:
    explicit ExecutionContext (StandardLayout::Mapping _recordMapping);

    void ExecuteTask (const CreateHashIndex &_task);

    void ExecuteTask (const CreateOrderedIndex &_task);

    void ExecuteTask (const CreateVolumetricIndex &_task);

    void ExecuteTask (const CopyIndexReference &_task);

    void ExecuteTask (const RemoveIndexReference &_task);

    void ExecuteTask (const CheckIsSourceBusy &_task);

    void ExecuteTask (const DropIndex &_task);

    void ExecuteTask (const OpenAllocator &);

    void ExecuteTask (const AllocateAndInit &_task);

    void ExecuteTask (const CloseAllocator &);

    void ExecuteTask (const QueryValueToRead &_task);

    void ExecuteTask (const QueryValueToEdit &_task);

    void ExecuteTask (const QueryRangeToRead &_task);

    void ExecuteTask (const QueryRangeToEdit &_task);

    void ExecuteTask (const QueryReversedRangeToRead &_task);

    void ExecuteTask (const QueryReversedRangeToEdit &_task);

    void ExecuteTask (const QueryShapeIntersectionToRead &_task);

    void ExecuteTask (const QueryShapeIntersectionToEdit &_task);

    void ExecuteTask (const QueryRayIntersectionToRead &_task);

    void ExecuteTask (const QueryRayIntersectionToEdit &_task);

    void ExecuteTask (const CursorCheck &_task);

    void ExecuteTask (const CursorCheckAllOrdered &_task);

    void ExecuteTask (const CursorCheckAllUnordered &_task);

    void ExecuteTask (const CursorEdit &_task);

    void ExecuteTask (const CursorIncrement &_task);

    void ExecuteTask (const CursorDeleteObject &_task);

    void ExecuteTask (const CursorCopy &_task);

    void ExecuteTask (const CursorMove &_task);

    void ExecuteTask (const CursorClose &_task);

private:
    const IndexReference &PrepareForLookup (const QueryBase &_task) const;

    void IterateOverIndices () const;

    void OnIndexDropped (HashIndex *_index);

    void OnIndexDropped (OrderedIndex *_index);

    void OnIndexDropped (VolumetricIndex *_index);

    VolumetricIndex::AxisAlignedShapeContainer ExtractShape (
        const VolumetricIndex *_index, const ShapeIntersectionQueryBase &_task) const;

    VolumetricIndex::RayContainer ExtractRay (
        const VolumetricIndex *_index, const RayIntersectionQueryBase &_task) const;

    Storage storage;
    std::vector <HashIndex *> knownHashIndices;
    std::vector <OrderedIndex *> knownOrderedIndices;
    std::vector <VolumetricIndex *> knownVolumetricIndices;

    std::unordered_map <std::string, IndexReference> indexReferences;
    std::optional <Storage::Allocator> storageAllocator;

    Query::Test::CursorManager <
        HashIndex::ReadCursor,
        HashIndex::EditCursor,
        OrderedIndex::ReadCursor,
        OrderedIndex::EditCursor,
        OrderedIndex::ReversedReadCursor,
        OrderedIndex::ReversedEditCursor,
        VolumetricIndex::ShapeIntersectionReadCursor,
        VolumetricIndex::ShapeIntersectionEditCursor,
        VolumetricIndex::RayIntersectionReadCursor,
        VolumetricIndex::RayIntersectionEditCursor> cursors;
};

ExecutionContext::ExecutionContext (StandardLayout::Mapping _recordMapping)
    : storage (std::move (_recordMapping)),
      knownHashIndices (),
      knownOrderedIndices (),
      knownVolumetricIndices (),
      indexReferences (),
      cursors ()
{
}

void ExecutionContext::ExecuteTask (const CreateHashIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    Handling::Handle <HashIndex> index = storage.CreateHashIndex (_task.indexedFields);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownHashIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateOrderedIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    Handling::Handle <OrderedIndex> index = storage.CreateOrderedIndex (_task.indexedField);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownOrderedIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CreateVolumetricIndex &_task)
{
    REQUIRE_WITH_MESSAGE (
        indexReferences.find (_task.name) == indexReferences.end (),
        "There should be no index reference with name \"", _task.name, "\".");

    std::vector <VolumetricIndex::DimensionDescriptor> convertedDescriptors;
    convertedDescriptors.reserve (_task.dimensions.size ());

    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        static_assert (sizeof (VolumetricIndex::SupportedAxisValue) == sizeof (dimension.globalMin));

        convertedDescriptors.emplace_back (
            VolumetricIndex::DimensionDescriptor
                {
                    *reinterpret_cast <const VolumetricIndex::SupportedAxisValue *> (&dimension.globalMin),
                    dimension.minField,
                    *reinterpret_cast <const VolumetricIndex::SupportedAxisValue *> (&dimension.globalMax),
                    dimension.maxField,
                });
    }

    Handling::Handle <VolumetricIndex> index = storage.CreateVolumetricIndex (convertedDescriptors);
    REQUIRE_WITH_MESSAGE (index, "Returned index should not be null.");

    knownVolumetricIndices.emplace_back (index.Get ());
    indexReferences.emplace (_task.name, index);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const CopyIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.sourceName, "\".");

    // Copying reference into itself is ok and may even be used as part of special test scenario.
    indexReferences.emplace (_task.targetName, iterator->second);
}

void ExecutionContext::ExecuteTask (const RemoveIndexReference &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    indexReferences.erase (iterator);
}

void ExecutionContext::ExecuteTask (const CheckIsSourceBusy &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    std::visit (
        [&_task] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Temporary make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;
            CHECK_EQUAL (handleValue->CanBeDropped (), !_task.expectedValue);
            _handle = handleValue;
        },
        iterator->second);
}

void ExecutionContext::ExecuteTask (const DropIndex &_task)
{
    auto iterator = indexReferences.find (_task.name);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.name, "\".");

    std::visit (
        [this] (auto &_handle)
        {
            auto *handleValue = _handle.Get ();
            // Firstly make handle free. Otherwise CanBeDropped check will always return false.
            _handle = nullptr;

            REQUIRE (handleValue->CanBeDropped ());
            handleValue->Drop ();
            OnIndexDropped (handleValue);
        },
        iterator->second);

    indexReferences.erase (iterator);
    IterateOverIndices ();
}

void ExecutionContext::ExecuteTask (const OpenAllocator &)
{
    REQUIRE_WITH_MESSAGE (!storageAllocator, "There should be no active allocator.");
    storageAllocator.emplace (storage.AllocateAndInsert ());
}

void ExecutionContext::ExecuteTask (const AllocateAndInit &_task)
{
    REQUIRE_WITH_MESSAGE (storageAllocator, "There should be active allocator.");
    void *record = storageAllocator.value ().Next ();
    CHECK (record != nullptr);

    if (record)
    {
        memcpy (record, _task.copyFrom, storage.GetRecordMapping ().GetObjectSize ());
    }
}

void ExecutionContext::ExecuteTask (const CloseAllocator &)
{
    REQUIRE_WITH_MESSAGE (storageAllocator, "There should be active allocator.");
    storageAllocator.reset ();
}

void ExecutionContext::ExecuteTask (const QueryValueToRead &_task)
{
    HashIndex *index = std::get <Handling::Handle <HashIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToRead ({_task.value}));
}

void ExecutionContext::ExecuteTask (const QueryValueToEdit &_task)
{
    HashIndex *index = std::get <Handling::Handle <HashIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToEdit ({_task.value}));
}

void ExecutionContext::ExecuteTask (const QueryRangeToRead &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToRead ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const QueryRangeToEdit &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToEdit ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToRead &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToReadReversed ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const QueryReversedRangeToEdit &_task)
{
    OrderedIndex *index = std::get <Handling::Handle <OrderedIndex>> (PrepareForLookup (_task)).Get ();
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupToEditReversed ({_task.minValue}, {_task.maxValue}));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToRead &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupShapeIntersectionToRead (ExtractShape (index, _task)));
}

void ExecutionContext::ExecuteTask (const QueryShapeIntersectionToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.min.size (), index->GetDimensions ().GetCount ());
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupShapeIntersectionToEdit (ExtractShape (index, _task)));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToRead &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupRayIntersectionToRead (ExtractRay (index, _task), _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const QueryRayIntersectionToEdit &_task)
{
    VolumetricIndex *index = std::get <Handling::Handle <VolumetricIndex>> (PrepareForLookup (_task)).Get ();
    REQUIRE_EQUAL (_task.origin.size (), index->GetDimensions ().GetCount ());
    cursors.Add (_task.cursorName, storage.GetRecordMapping (),
                 index->LookupRayIntersectionToEdit (ExtractRay (index, _task), _task.maxDistance));
}

void ExecutionContext::ExecuteTask (const CursorCheck &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllOrdered &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCheckAllUnordered &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorEdit &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorIncrement &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorDeleteObject &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorCopy &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorMove &_task)
{
    cursors.ExecuteTask (_task);
}

void ExecutionContext::ExecuteTask (const CursorClose &_task)
{
    cursors.ExecuteTask (_task);
}

const IndexReference &ExecutionContext::PrepareForLookup (const QueryBase &_task) const
{
    auto iterator = indexReferences.find (_task.sourceName);
    REQUIRE_WITH_MESSAGE (
        iterator != indexReferences.end (),
        "There should be index reference with name \"", _task.sourceName, "\".");

    return iterator->second;
}

void ExecutionContext::IterateOverIndices () const
{
    auto Iterate = [] (auto _current, auto _end, const auto &_known)
    {
        std::decay_t <decltype (_known)> found;
        while (_current != _end)
        {
            found.emplace_back ((*_current).Get ());
            ++_current;
        }

        CHECK_EQUAL (found.size (), _known.size ());
        for (const auto &index : found)
        {
            CHECK_WITH_MESSAGE (std::find (_known.begin (), _known.end (), index) != _known.end (),
                                "Searching received index with address ", index, " in known list.");
        }

        for (const auto &index : _known)
        {
            CHECK_WITH_MESSAGE (std::find (found.begin (), found.end (), index) != found.end (),
                                "Searching known index with address ", index, " in received list.");
        }
    };

    LOG ("Checking hash indices.");
    Iterate (storage.BeginHashIndices (), storage.EndHashIndices (), knownHashIndices);

    LOG ("Checking ordered indices.");
    Iterate (storage.BeginOrderedIndices (), storage.EndOrderedIndices (), knownOrderedIndices);

    LOG ("Checking volumetric indices.");
    Iterate (storage.BeginVolumetricIndices (), storage.EndVolumetricIndices (), knownVolumetricIndices);
}

void ExecutionContext::OnIndexDropped (HashIndex *_index)
{
    auto iterator = std::find (knownHashIndices.begin (), knownHashIndices.end (), _index);
    CHECK (iterator != knownHashIndices.end ());

    if (iterator != knownHashIndices.end ())
    {
        knownHashIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (OrderedIndex *_index)
{
    auto iterator = std::find (knownOrderedIndices.begin (), knownOrderedIndices.end (), _index);
    CHECK (iterator != knownOrderedIndices.end ());

    if (iterator != knownOrderedIndices.end ())
    {
        knownOrderedIndices.erase (iterator);
    }
}

void ExecutionContext::OnIndexDropped (VolumetricIndex *_index)
{
    auto iterator = std::find (knownVolumetricIndices.begin (), knownVolumetricIndices.end (), _index);
    CHECK (iterator != knownVolumetricIndices.end ());

    if (iterator != knownVolumetricIndices.end ())
    {
        knownVolumetricIndices.erase (iterator);
    }
}

template <typename Callback>
auto WithPrimitiveFieldType (const StandardLayout::Field &_field, const Callback &_callback)
{
    REQUIRE (_field.IsHandleValid ());
    switch (_field.GetArchetype ())
    {
        case StandardLayout::FieldArchetype::INT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (int8_t):
                    return _callback (int8_t {});

                case sizeof (int16_t):
                    return _callback (int16_t {});

                case sizeof (int32_t):
                    return _callback (int32_t {});

                case sizeof (int64_t):
                    return _callback (int64_t {});
            }

            break;
        }
        case StandardLayout::FieldArchetype::UINT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (uint8_t):
                    return _callback (uint8_t {});

                case sizeof (uint16_t):
                    return _callback (uint16_t {});

                case sizeof (uint32_t):
                    return _callback (uint32_t {});

                case sizeof (uint64_t):
                    return _callback (uint64_t {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::FLOAT:
        {
            switch (_field.GetSize ())
            {
                case sizeof (float):
                    return _callback (float {});

                case sizeof (double):
                    return _callback (double {});
            }

            break;
        }

        case StandardLayout::FieldArchetype::BIT:
        case StandardLayout::FieldArchetype::BLOCK:
        case StandardLayout::FieldArchetype::NESTED_OBJECT:
        case StandardLayout::FieldArchetype::STRING:
        {
            break;
        }
    }

    REQUIRE_WITH_MESSAGE(false, "Field type should be primitive!");
    return _callback (float {});
}

VolumetricIndex::AxisAlignedShapeContainer ExecutionContext::ExtractShape (
    const VolumetricIndex *_index, const ShapeIntersectionQueryBase &_task) const
{
    REQUIRE_EQUAL (_task.min.size (), _task.max.size ());
    REQUIRE (_task.min.size () > 0u);
    REQUIRE (_task.min.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    REQUIRE (_index);
    REQUIRE (!_index->GetDimensions ().Empty ());

    return WithPrimitiveFieldType (
        _index->GetDimensions ()[0u].minBorderField,
        [&_task] (auto _typeInstance) -> VolumetricIndex::AxisAlignedShapeContainer
        {
            using ValueType = std::decay_t <decltype (_typeInstance)>;
            VolumetricIndex::AxisAlignedShape <ValueType> shape {};

            for (std::size_t dimensionIndex = 0u; dimensionIndex < _task.min.size (); ++dimensionIndex)
            {
                shape.Min (dimensionIndex) = *reinterpret_cast<const ValueType *> (&_task.min[dimensionIndex]);
                shape.Max (dimensionIndex) = *reinterpret_cast<const ValueType *> (&_task.max[dimensionIndex]);
            }

            return *reinterpret_cast <VolumetricIndex::AxisAlignedShapeContainer *> (&shape);
        });
}

VolumetricIndex::RayContainer ExecutionContext::ExtractRay (
    const VolumetricIndex *_index, const RayIntersectionQueryBase &_task) const
{
    REQUIRE_EQUAL (_task.origin.size (), _task.direction.size ());
    REQUIRE (_task.origin.size () > 0u);
    REQUIRE (_task.origin.size () <= Constants::VolumetricIndex::MAX_DIMENSIONS);

    REQUIRE (_index);
    REQUIRE (!_index->GetDimensions ().Empty ());

    return WithPrimitiveFieldType (
        _index->GetDimensions ()[0u].minBorderField,
        [&_task] (auto _typeInstance) -> VolumetricIndex::RayContainer
        {
            using ValueType = std::decay_t <decltype (_typeInstance)>;
            VolumetricIndex::Ray <ValueType> ray {};

            for (std::size_t dimensionIndex = 0u; dimensionIndex < _task.origin.size (); ++dimensionIndex)
            {
                ray.Origin (dimensionIndex) = *reinterpret_cast<const ValueType *> (&_task.origin[dimensionIndex]);
                ray.Direction (dimensionIndex) =
                    *reinterpret_cast<const ValueType *> (&_task.direction[dimensionIndex]);
            }

            return *reinterpret_cast <VolumetricIndex::RayContainer *> (&ray);
        });
}

std::ostream &operator << (std::ostream &_output, const CreateHashIndex &_task)
{
    _output << "Create hash index \"" << _task.name << "\" on fields:";
    for (StandardLayout::FieldId fieldId : _task.indexedFields)
    {
        _output << " " << fieldId;
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CreateOrderedIndex &_task)
{
    return _output << "Create ordered index \"" << _task.name << "\" on field " << _task.indexedField << ".";
}

std::ostream &operator << (std::ostream &_output, const CreateVolumetricIndex &_task)
{
    _output << "Create volumetric index \"" << _task.name << "\" on dimensions:";
    for (const Query::Test::Sources::Volumetric::Dimension &dimension : _task.dimensions)
    {
        _output << " { globalMin: " << dimension.globalMin << ", minField: " <<
                dimension.minField << ", globalMax: " << dimension.globalMax <<
                ", maxField: " << dimension.maxField << " }";
    }

    return _output << ".";
}

std::ostream &operator << (std::ostream &_output, const CopyIndexReference &_task)
{
    return _output << "Copy index reference \"" << _task.sourceName << "\" to \"" << _task.targetName << "\".";
}

std::ostream &operator << (std::ostream &_output, const RemoveIndexReference &_task)
{
    return _output << "Remove index reference \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const DropIndex &_task)
{
    return _output << "Drop index \"" << _task.name << "\".";
}

std::ostream &operator << (std::ostream &_output, const OpenAllocator &)
{
    return _output << "Open allocator.";
}

std::ostream &operator << (std::ostream &_output, const AllocateAndInit &_task)
{
    return _output << "Allocate record and init from " << _task.copyFrom << ".";
}

std::ostream &operator << (std::ostream &_output, const CloseAllocator &)
{
    return _output << "Close allocator.";
}

static void ExecuteQueryApiScenario (const Query::Test::Scenario &_scenario, bool _insertFirst)
{
    std::vector <Task> tasks;
    REQUIRE_WITH_MESSAGE (
        _scenario.storages.size () == 1u,
        "Only one-storage tests are supported right now, because Pegasus storages are independent.");

    auto InsertRecords = [&_scenario, &tasks] ()
    {
        tasks.emplace_back (OpenAllocator {});
        for (const void *record : _scenario.storages[0u].objectsToInsert)
        {
            tasks.emplace_back (AllocateAndInit {record});
        }

        tasks.emplace_back (CloseAllocator {});
    };

    auto CreateIndices = [&_scenario, &tasks] ()
    {
        for (const Query::Test::Source &source : _scenario.storages[0u].sources)
        {
            std::visit (
                [&tasks] (const auto &_unwrappedSource)
                {
                    using Source = std::decay_t <decltype (_unwrappedSource)>;
                    if constexpr (std::is_same_v <Source, Query::Test::Sources::Value>)
                    {
                        tasks.emplace_back (CreateHashIndex {_unwrappedSource.name, _unwrappedSource.queriedFields});
                    }
                    else if constexpr (std::is_same_v <Source, Query::Test::Sources::Range>)
                    {
                        tasks.emplace_back (CreateOrderedIndex {_unwrappedSource.name, _unwrappedSource.queriedField});
                    }
                    else if constexpr (std::is_same_v <Source, Query::Test::Sources::Volumetric>)
                    {
                        tasks.emplace_back (CreateVolumetricIndex {_unwrappedSource.name, _unwrappedSource.dimensions});
                    }
                    else
                    {
                        REQUIRE_WITH_MESSAGE (false, "Only Value, Range and Volumetric sources are supported!");
                    }
                },
                source);
        }
    };

    if (_insertFirst)
    {
        InsertRecords ();
        CreateIndices ();
    }
    else
    {
        CreateIndices ();
        InsertRecords ();
    }

    for (const Query::Test::Task &task : _scenario.tasks)
    {
        std::visit (
            [&tasks] (const auto &_unwrappedTask)
            {
                using TaskType = std::decay_t <decltype (_unwrappedTask)>;
                if constexpr (std::is_same_v <TaskType, QuerySingletonToRead> ||
                              std::is_same_v <TaskType, QuerySingletonToEdit> ||
                              std::is_same_v <TaskType, QueryUnorderedSequenceToRead> ||
                              std::is_same_v <TaskType, QueryUnorderedSequenceToEdit>)
                {
                    REQUIRE_WITH_MESSAGE (false, "Singleton and unordered sequence queries are not supported!");
                }
                else
                {
                    tasks.emplace_back (_unwrappedTask);
                }
            },
            task);
    }

    Scenario (_scenario.storages[0u].dataType, tasks);
}

void TestQueryApiDrivers::CreateIndicesThanInsertRecords (const Query::Test::Scenario &_scenario)
{
    ExecuteQueryApiScenario (_scenario, false);
}

void TestQueryApiDrivers::InsertRecordsThanCreateIndices (const Query::Test::Scenario &_scenario)
{
    ExecuteQueryApiScenario (_scenario, true);
}

Scenario::Scenario (StandardLayout::Mapping _mapping, std::vector <Task> _tasks)
    : mapping (std::move (_mapping)),
      tasks (std::move (_tasks))
{
    ExecutionContext context (mapping);
    LOG ((std::stringstream () << *this).str ());

    for (const Task &wrappedTask : tasks)
    {
        std::visit (
            [&context] (const auto &_unwrappedTask)
            {
                LOG ((std::stringstream () << _unwrappedTask).str ());
                context.ExecuteTask (_unwrappedTask);
            },
            wrappedTask);
    }
}

std::ostream &operator << (std::ostream &_output, const Scenario &_scenario)
{
    _output << "Scenario: " << std::endl;
    for (const Task &wrappedTask : _scenario.tasks)
    {
        _output << " - ";
        std::visit (
            [&_output] (const auto &_unwrappedTask)
            {
                _output << _unwrappedTask;
            },
            wrappedTask);

        _output << std::endl;
    }

    return _output;
}

std::vector <Task> operator + (std::vector <Task> first, const std::vector <Task> &second) noexcept
{
    first.insert (first.end (), second.begin (), second.end ());
    return first;
}
} // namespace Emergence::Pegasus::Test