#include <Celerity/Assembly/AssemblerConfiguration.hpp>
#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Container/HashMap.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::Assembly
{
const Memory::UniqueString Checkpoint::STARTED {"AssemblyStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"AssemblyFinished"};

class AssemblerBase
{
public:
    AssemblerBase (TaskConstructor &_constructor,
                   const CustomKeyVector &_customKeys,
                   const TypeBindingVector &_types,
                   const StandardLayout::Mapping &_finishedEventType) noexcept;

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

    ModifyValueQuery modifyPrototypeById;
    FetchValueQuery fetchDescriptorById;
    FetchValueQuery fetchTransformById;
    Transform3dWorldAccessor transformWorldAccessor;
    InsertShortTermQuery insertFinishedEvent;

    // TODO: Use flat hash map.
    Container::HashMap<StandardLayout::Mapping, TypeBinding> typeBindings {Memory::Profiler::AllocationGroup::Top ()};

    Container::Vector<KeyState> keyStates {Memory::Profiler::AllocationGroup::Top ()};

    bool needRootObjectTransform = false;
};

static UniqueId WorldObjectIdProvider (const void *_singleton)
{
    return static_cast<const WorldSingleton *> (_singleton)->GenerateId ();
}

AssemblerBase::AssemblerBase (TaskConstructor &_constructor,
                              const CustomKeyVector &_customKeys,
                              const TypeBindingVector &_types,
                              const StandardLayout::Mapping &_finishedEventType) noexcept
    : modifyPrototypeById (MODIFY_VALUE_1F (PrototypeComponent, objectId)),
      fetchDescriptorById (FETCH_VALUE_1F (AssemblyDescriptor, id)),
      fetchTransformById (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),
      insertFinishedEvent (_constructor.InsertShortTerm (_finishedEventType))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);

    for (const CustomKeyDescriptor &customKey : _customKeys)
    {
        keyStates.emplace_back (
            KeyState {_constructor.FetchSingleton (customKey.singletonProviderType), customKey.providerFunction,
                      Container::HashMap<UniqueId, UniqueId> {Memory::Profiler::AllocationGroup::Top ()}});
    }

    keyStates.emplace_back (
        KeyState {FETCH_SINGLETON (WorldSingleton), WorldObjectIdProvider,
                  Container::HashMap<UniqueId, UniqueId> {Memory::Profiler::AllocationGroup::Top ()}});

    for (const TypeDescriptor &typeDescriptor : _types)
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
    auto prototypeCursor = modifyPrototypeById.Execute (&_rootObjectId);
    auto *prototype = static_cast<PrototypeComponent *> (*prototypeCursor);

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

    // See PrototypeComponent::intermediateIdReplacement for details.
    bool idReplacementInherited = false;

    if (!prototype->intermediateIdReplacement.empty ())
    {
        // Inherit intermediate id replacement from previous pass during other routine.
        EMERGENCE_ASSERT (prototype->intermediateIdReplacement.size () == keyStates.size ());

        for (std::size_t index = 0u; index < keyStates.size (); ++index)
        {
            for (const auto &[from, to] : prototype->intermediateIdReplacement[index])
            {
                keyStates[index].idReplacement.emplace (from, to);
            }
        }

        idReplacementInherited = true;
        prototype->intermediateIdReplacement.clear ();
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
                EMERGENCE_ASSERT (needRootObjectTransform);
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

    if (idReplacementInherited)
    {
        // If id replacement was inherited then we reached last assembly step and
        // no longer need to keep component around after fully assembling object.
        ~prototypeCursor;
    }
    else
    {
        prototype->intermediateIdReplacement.resize (
            keyStates.size (),
            Container::HashMap<UniqueId, UniqueId> {prototype->intermediateIdReplacement.get_allocator ()});

        for (std::size_t index = 0u; index < keyStates.size (); ++index)
        {
            for (const auto &[from, to] : keyStates[index].idReplacement)
            {
                prototype->intermediateIdReplacement[index].emplace (from, to);
            }
        }
    }

    for (auto &keyState : keyStates)
    {
        keyState.idReplacement.clear ();
    }

    auto eventCursor = insertFinishedEvent.Execute ();
    void *event = ++eventCursor;
    *static_cast<UniqueId *> (event) = _rootObjectId;
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

    EMERGENCE_ASSERT (_index < keyStates.size () - 1u);
    return keyStates[_index];
}

class FixedAssembler final : public TaskExecutorBase<FixedAssembler>, public AssemblerBase
{
public:
    FixedAssembler (TaskConstructor &_constructor,
                    const CustomKeyVector &_customKeys,
                    const TypeBindingVector &_types) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchPrototypeAddedFixedEvents;
    FetchSequenceQuery fetchPrototypeAddedCustomToFixedEvents;
};

FixedAssembler::FixedAssembler (TaskConstructor &_constructor,
                                const CustomKeyVector &_customKeys,
                                const TypeBindingVector &_types) noexcept
    : AssemblerBase (_constructor, _customKeys, _types, AssemblyFinishedFixedEvent::Reflect ().mapping),
      fetchPrototypeAddedFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedEvent)),
      fetchPrototypeAddedCustomToFixedEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToFixedEvent))
{
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
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
    NormalAssembler (TaskConstructor &_constructor,
                     const CustomKeyVector &_customKeys,
                     const TypeBindingVector &_types) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchPrototypeAddedNormalEvents;
    FetchSequenceQuery fetchPrototypeAddedFixedToNormalEvents;
    FetchSequenceQuery fetchPrototypeAddedCustomToNormalEvents;
};

NormalAssembler::NormalAssembler (TaskConstructor &_constructor,
                                  const CustomKeyVector &_customKeys,
                                  const TypeBindingVector &_types) noexcept
    : AssemblerBase (_constructor, _customKeys, _types, AssemblyFinishedNormalEvent::Reflect ().mapping),
      fetchPrototypeAddedNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedNormalEvent)),
      fetchPrototypeAddedFixedToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedFixedToNormalEvent)),
      fetchPrototypeAddedCustomToNormalEvents (FETCH_SEQUENCE (PrototypeComponentAddedCustomToNormalEvent))
{
    useLogicalTransform = false;
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (TransformVisualSync::Checkpoint::STARTED);
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

static void AddCheckpoints (PipelineBuilder &_pipelineBuilder)
{
    _pipelineBuilder.AddCheckpoint (Assembly::Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Assembly::Checkpoint::FINISHED);
}

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder,
                       const CustomKeyVector &_allCustomKeys,
                       const TypeBindingVector &_fixedUpdateTypes) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, PrototypeComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Assembly");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask ("Assembly::FixedUpdate"_us)
        .SetExecutor<FixedAssembler> (_allCustomKeys, _fixedUpdateTypes);
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const CustomKeyVector &_allCustomKeys,
                        const TypeBindingVector &_normalUpdateTypes) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, PrototypeComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    // We don't care about fixed-to-normal transform removal events,
    // because in this case prototype will be removed through the special task in fixed update.

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Assembly");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask ("Assembly::NormalUpdate"_us)
        .SetExecutor<NormalAssembler> (_allCustomKeys, _normalUpdateTypes);
}
} // namespace Emergence::Celerity::Assembly
