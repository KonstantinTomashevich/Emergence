#include <Celerity/Assembly/AssemblerConfiguration.hpp>
#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>
#include <Celerity/Transform/Transform3dWorldAccessor.hpp>

#include <Container/HashMap.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::Assembly
{
const Emergence::Memory::UniqueString Checkpoint::ASSEMBLY_STARTED {"AssemblyStarted"};
const Emergence::Memory::UniqueString Checkpoint::ASSEMBLY_FINISHED {"AssemblyFinished"};

class AssemblerBase
{
public:
    AssemblerBase (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept;

protected:
    void AssembleObject (UniqueId _rootObjectId) noexcept;

    bool useLogicalTransform = true;

private:
    struct InternalKeyBinding final
    {
        StandardLayout::Field keyField;
        UniqueId keyIndex;
    };

    struct TypeBinding final
    {
        InsertLongTermQuery insert;
        Container::Vector<InternalKeyBinding> keys {Memory::Profiler::AllocationGroup::Top ()};
        Container::Vector<StandardLayout::Field> rotateVector3fs {Memory::Profiler::AllocationGroup::Top ()};
    };

    struct KeyState final
    {
        using ProviderFunction = UniqueId (*) (const void *);

        FetchSingletonQuery fetchSingleton;
        ProviderFunction function;

        // TODO: Use flat hash map. Possibly use some inlining?
        Container::HashMap<UniqueId, UniqueId> idReplacement;
    };

    static UniqueId ReplaceId (KeyState &_keyState, UniqueId _id) noexcept;

    KeyState &GetObjectIdKeyState () noexcept;

    KeyState &GetKeyState (UniqueId _index) noexcept;

    FetchValueQuery fetchDescriptorById;
    FetchValueQuery fetchPrototypeById;
    FetchValueQuery fetchTransformById;
    Transform3dWorldAccessor transformWorldAccessor;

    // TODO: Use flat hash map.
    Container::HashMap<StandardLayout::Mapping, TypeBinding> typeBindings {Memory::Profiler::AllocationGroup::Top ()};

    Container::Vector<KeyState> keyStates {Memory::Profiler::AllocationGroup::Top ()};

    bool needRootObjectTransform = false;
};

static UniqueId WorldObjectIdProvider (const void *_singleton)
{
    return static_cast<const WorldSingleton *> (_singleton)->GenerateId ();
}

AssemblerBase::AssemblerBase (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept
    : fetchDescriptorById (FETCH_VALUE_1F (AssemblyDescriptor, id)),
      fetchPrototypeById (FETCH_VALUE_1F (PrototypeComponent, objectId)),
      fetchTransformById (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSEMBLY_FINISHED);

    for (const CustomKeyDescriptor &customKey : _configuration.customKeys)
    {
        keyStates.emplace_back (
            KeyState {_constructor.FetchSingleton (customKey.singletonProviderType), customKey.providerFunction,
                      Container::HashMap<UniqueId, UniqueId> {Memory::Profiler::AllocationGroup::Top ()}});
    }

    keyStates.emplace_back (
        KeyState {FETCH_SINGLETON (WorldSingleton), WorldObjectIdProvider,
                  Container::HashMap<UniqueId, UniqueId> {Memory::Profiler::AllocationGroup::Top ()}});

    for (const TypeDescriptor &typeDescriptor : _configuration.types)
    {
        TypeBinding &binding =
            typeBindings.emplace (typeDescriptor.type, TypeBinding {_constructor.InsertLongTerm (typeDescriptor.type)})
                .first->second;

        for (const KeyBinding &keyBinding : typeDescriptor.keys)
        {
            binding.keys.emplace_back () = {typeDescriptor.type.GetField (keyBinding.keyField), keyBinding.keyIndex};
        }

        needRootObjectTransform |= !typeDescriptor.rotateVector3fs.empty ();
        for (const StandardLayout::FieldId &vectorField : typeDescriptor.rotateVector3fs)
        {
            binding.rotateVector3fs.emplace_back (typeDescriptor.type.GetField (vectorField));
        }
    }
}

void AssemblerBase::AssembleObject (UniqueId _rootObjectId) noexcept
{
    auto prototypeCursor = fetchPrototypeById.Execute (&_rootObjectId);
    const auto *prototype = static_cast<const PrototypeComponent *> (*prototypeCursor);

    if (!prototype)
    {
        EMERGENCE_LOG (ERROR, "Assembly: Unable to assemble object with id ", _rootObjectId,
                       ", because it has no PrototypeComponent!");
        return;
    }

    Math::Transform3d rootObjectTransform;
    if (needRootObjectTransform)
    {
        auto transformCursor = fetchTransformById.Execute (&_rootObjectId);
        if (const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor))
        {
            rootObjectTransform = useLogicalTransform ? transform->GetLogicalWorldTransform (transformWorldAccessor) :
                                                        transform->GetVisualWorldTransform (transformWorldAccessor);
        }
        else
        {
            EMERGENCE_LOG (ERROR, "Assembly: Unable to assemble object with id ", _rootObjectId,
                           ", because it has no Transform3dComponent!");
            return;
        }
    }

    auto descriptorCursor = fetchDescriptorById.Execute (&prototype->descriptorId);
    const auto *descriptor = static_cast<const AssemblyDescriptor *> (*descriptorCursor);

    if (!descriptor)
    {
        EMERGENCE_LOG (ERROR, "Assembly: Unable to find AssemblyDescriptor with id \"", prototype->descriptorId, "\"!");
        return;
    }

    GetObjectIdKeyState ().idReplacement.emplace (ASSEMBLY_ROOT_OBJECT_ID, _rootObjectId);
    for (const StandardLayout::Patch &componentDescriptor : descriptor->components)
    {
        auto iterator = typeBindings.find (componentDescriptor.GetTypeMapping ());
        if (iterator != typeBindings.end ())
        {
            TypeBinding &binding = iterator->second;
            auto insertionCursor = binding.insert.Execute ();
            void *component = ++insertionCursor;
            componentDescriptor.Apply (component);

            for (const InternalKeyBinding &keyBinding : binding.keys)
            {
                KeyState &keyState = GetKeyState (keyBinding.keyIndex);
                auto *id = static_cast<UniqueId *> (keyBinding.keyField.GetValue (component));

                if (*id != INVALID_UNIQUE_ID)
                {
                    *id = ReplaceId (keyState, *id);
                }
            }

            for (const StandardLayout::Field &vectorField : binding.rotateVector3fs)
            {
                assert (needRootObjectTransform);
                auto *vector = static_cast<Math::Vector3f *> (vectorField.GetValue (component));
                *vector = Math::Rotate (*vector, rootObjectTransform.rotation);
            }
        }
        else
        {
            EMERGENCE_LOG (DEBUG, "Skipping assembly of unknown type \"",
                           componentDescriptor.GetTypeMapping ().GetName (), "\"...");
        }
    }

    for (KeyState &state : keyStates)
    {
        state.idReplacement.clear ();
    }
}

UniqueId AssemblerBase::ReplaceId (AssemblerBase::KeyState &_keyState, UniqueId _id) noexcept
{
    auto iterator = _keyState.idReplacement.find (_id);
    if (iterator == _keyState.idReplacement.end ())
    {
        auto singletonCursor = _keyState.fetchSingleton.Execute ();
        UniqueId newId = _keyState.function (*singletonCursor);
        _keyState.idReplacement.emplace (_id, newId);
        return newId;
    }

    return iterator->second;
}

AssemblerBase::KeyState &AssemblerBase::GetObjectIdKeyState () noexcept
{
    return keyStates.back ();
}

AssemblerBase::KeyState &AssemblerBase::GetKeyState (UniqueId _index) noexcept
{
    if (_index == ASSEMBLY_OBJECT_ID_KEY_INDEX)
    {
        return GetObjectIdKeyState ();
    }

    assert (_index < keyStates.size () - 1u);
    return keyStates[_index];
}

class FixedAssembler final : public TaskExecutorBase<FixedAssembler>, public AssemblerBase
{
public:
    FixedAssembler (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchPrototypeAddedFixedEvents;
    FetchSequenceQuery fetchPrototypeAddedCustomToFixedEvents;
};

FixedAssembler::FixedAssembler (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept
    : AssemblerBase (_constructor, _configuration),
      fetchPrototypeAddedFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedEvent)),
      fetchPrototypeAddedCustomToFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToFixedEvent))
{
}

void FixedAssembler::Execute () noexcept
{
    for (auto eventCursor = fetchPrototypeAddedFixedEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedFixedEvent *> (*eventCursor); ++eventCursor)
    {
        AssembleObject (event->objectId);
    }

    for (auto eventCursor = fetchPrototypeAddedCustomToFixedEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedCustomToFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        AssembleObject (event->objectId);
    }
}

class NormalAssembler final : public TaskExecutorBase<NormalAssembler>, public AssemblerBase
{
public:
    NormalAssembler (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchPrototypeAddedNormalEvents;
    FetchSequenceQuery fetchPrototypeAddedFixedToNormalEvents;
    FetchSequenceQuery fetchPrototypeAddedCustomToNormalEvents;
};

NormalAssembler::NormalAssembler (TaskConstructor &_constructor, const AssemblerConfiguration &_configuration) noexcept
    : AssemblerBase (_constructor, _configuration),
      fetchPrototypeAddedNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedNormalEvent)),
      fetchPrototypeAddedFixedToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedToNormalEvent)),
      fetchPrototypeAddedCustomToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToNormalEvent))
{
    useLogicalTransform = false;
    _constructor.DependOn (VisualTransformSync::Checkpoint::SYNC_FINISHED);
}

void NormalAssembler::Execute () noexcept
{
    for (auto eventCursor = fetchPrototypeAddedNormalEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        AssembleObject (event->objectId);
    }

    for (auto eventCursor = fetchPrototypeAddedFixedToNormalEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        AssembleObject (event->objectId);
    }

    for (auto eventCursor = fetchPrototypeAddedCustomToNormalEvents.Execute ();
         const auto *event = static_cast<const PrototypeComponentAddedCustomToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        AssembleObject (event->objectId);
    }
}

using namespace Memory::Literals;

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder, const AssemblerConfiguration &_configuration) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (Transform3dComponentRemovedFixedEvent, PrototypeComponent, objectId)
        .DependOn (Checkpoint::ASSEMBLY_STARTED)
        .MakeDependencyOf ("Assembly::FixedUpdate"_us);

    _pipelineBuilder.AddTask ("Assembly::FixedUpdate"_us).SetExecutor<FixedAssembler> (_configuration);
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const AssemblerConfiguration &_configuration) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (Transform3dComponentRemovedNormalEvent, PrototypeComponent, objectId)
        .DependOn (Checkpoint::ASSEMBLY_STARTED)
        .MakeDependencyOf ("Assembly::NormalUpdate"_us);

    // We don't care about fixed-to-normal transform removal events,
    // because in this case prototype will be removed through the special task in fixed update.

    _pipelineBuilder.AddTask ("Assembly::NormalUpdate"_us).SetExecutor<NormalAssembler> (_configuration);
}
} // namespace Emergence::Celerity::Assembly
