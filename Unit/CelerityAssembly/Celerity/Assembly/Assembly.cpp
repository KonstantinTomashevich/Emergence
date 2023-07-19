#include <Celerity/Assembly/AssemblerConfiguration.hpp>
#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeAssemblyComponent.hpp>
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

#include <Time/Time.hpp>

namespace Emergence::Celerity::Assembly
{
enum class AssemblyExecutionResult
{
    DONE,
    PARTIAL,
    OUT_OF_TIME,
};

const Memory::UniqueString Checkpoint::STARTED {"AssemblyStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"AssemblyFinished"};

class Assembler final : public TaskExecutorBase<Assembler>
{
public:
    Assembler (TaskConstructor &_constructor,
               const CustomKeyVector &_customKeys,
               const TypeBindingVector &_types,
               bool _isFixed,
               std::uint64_t _assemblyTimeLimit) noexcept;

    void Execute () noexcept;

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

    void StartFreshPrototypeAssembly () noexcept;

    void ProcessImmediatePrototypes () noexcept;

    void ProcessWaitingPrototypes (std::uint64_t _executionStartTime) noexcept;

    AssemblyExecutionResult AssembleObject (PrototypeAssemblyComponent *_assembly,
                                            std::uint64_t _executionStartTime) noexcept;

    static UniqueId ReplaceId (KeyState &_keyState, UniqueId _id) noexcept;

    KeyState &GetObjectIdKeyState () noexcept;

    KeyState &GetKeyState (UniqueId _index) noexcept;

    FetchValueQuery fetchPrototypeById;
    FetchValueQuery fetchDescriptorById;

    FetchSignalQuery fetchFreshPrototypes;
    ModifySignalQuery modifyFreshPrototypes;

    FetchSignalQuery fetchImmediatePrototypeAssemblies;
    ModifySignalQuery modifyImmediatePrototypeAssemblies;

    FetchSignalQuery fetchWaitingPrototypeAssemblies;
    ModifySignalQuery modifyWaitingPrototypeAssemblies;

    FetchValueQuery fetchTransform3dById;
    Transform3dWorldAccessor transformWorldAccessor;

    InsertLongTermQuery insertPrototypeAssembly;
    InsertShortTermQuery insertFinishedEvent;

    // TODO: Use flat hash map.
    Container::HashMap<StandardLayout::Mapping, TypeBinding> typeBindings {Memory::Profiler::AllocationGroup::Top ()};

    Container::Vector<KeyState> keyStates {Memory::Profiler::AllocationGroup::Top ()};

    const std::uint64_t assemblyTimeLimit;
    const bool isFixed;
    bool needRootObjectTransform3d = false;
};

[[maybe_unused]] static UniqueId WorldObjectIdProvider (const void *_singleton)
{
    return static_cast<const WorldSingleton *> (_singleton)->GenerateId ();
}

Assembler::Assembler (TaskConstructor &_constructor,
                      const CustomKeyVector &_customKeys,
                      const TypeBindingVector &_types,
                      bool _isFixed,
                      std::uint64_t _assemblyTimeLimit) noexcept
    : TaskExecutorBase (_constructor),

      fetchPrototypeById (FETCH_VALUE_1F (PrototypeComponent, objectId)),
      fetchDescriptorById (FETCH_VALUE_1F (AssemblyDescriptor, id)),

      fetchFreshPrototypes (FETCH_SIGNAL (PrototypeComponent, assemblyStarted, false)),
      modifyFreshPrototypes (MODIFY_SIGNAL (PrototypeComponent, assemblyStarted, false)),

      fetchImmediatePrototypeAssemblies (_constructor.FetchSignal (
          PrototypeAssemblyComponent::Reflect ().mapping,
          _isFixed ? PrototypeAssemblyComponent::Reflect ().fixedAssemblyState :
                     PrototypeAssemblyComponent::Reflect ().normalAssemblyState,
          array_cast<AssemblyState, sizeof (std::uint64_t)> (AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY))),
      modifyImmediatePrototypeAssemblies (_constructor.ModifySignal (
          PrototypeAssemblyComponent::Reflect ().mapping,
          _isFixed ? PrototypeAssemblyComponent::Reflect ().fixedAssemblyState :
                     PrototypeAssemblyComponent::Reflect ().normalAssemblyState,
          array_cast<AssemblyState, sizeof (std::uint64_t)> (AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY))),

      fetchWaitingPrototypeAssemblies (_constructor.FetchSignal (
          PrototypeAssemblyComponent::Reflect ().mapping,
          _isFixed ? PrototypeAssemblyComponent::Reflect ().fixedAssemblyState :
                     PrototypeAssemblyComponent::Reflect ().normalAssemblyState,
          array_cast<AssemblyState, sizeof (std::uint64_t)> (AssemblyState::WAITING_FOR_ASSEMBLY))),
      modifyWaitingPrototypeAssemblies (_constructor.ModifySignal (
          PrototypeAssemblyComponent::Reflect ().mapping,
          _isFixed ? PrototypeAssemblyComponent::Reflect ().fixedAssemblyState :
                     PrototypeAssemblyComponent::Reflect ().normalAssemblyState,
          array_cast<AssemblyState, sizeof (std::uint64_t)> (AssemblyState::WAITING_FOR_ASSEMBLY))),

      fetchTransform3dById (FETCH_VALUE_1F (Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      insertPrototypeAssembly (INSERT_LONG_TERM (PrototypeAssemblyComponent)),
      insertFinishedEvent (_constructor.InsertShortTerm (_isFixed ? AssemblyFinishedFixedEvent::Reflect ().mapping :
                                                                    AssemblyFinishedNormalEvent::Reflect ().mapping)),

      assemblyTimeLimit (_assemblyTimeLimit),
      isFixed (_isFixed)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);

    if (!isFixed)
    {
        _constructor.MakeDependencyOf (TransformVisualSync::Checkpoint::STARTED);
    }

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

        needRootObjectTransform3d |= !typeDescriptor.rotateVector3fs.empty ();
        for (const StandardLayout::FieldId &vectorField : typeDescriptor.rotateVector3fs)
        {
            binding.rotateVector3fs.emplace_back (typeDescriptor.type.GetField (vectorField));
        }
    }
}

void Assembler::Execute () noexcept
{
    const std::uint64_t startTime = Time::NanosecondsSinceStartup ();
    bool hasUninitializedPrototypes = *fetchFreshPrototypes.Execute ();
    bool hasPendingImmediatePrototypes = *fetchImmediatePrototypeAssemblies.Execute ();
    bool hasPendingWaitingPrototypes = *fetchWaitingPrototypeAssemblies.Execute ();

    while (hasUninitializedPrototypes || hasPendingImmediatePrototypes ||
           (hasPendingWaitingPrototypes && Time::NanosecondsSinceStartup () - startTime < assemblyTimeLimit))
    {
        StartFreshPrototypeAssembly ();
        ProcessImmediatePrototypes ();
        ProcessWaitingPrototypes (startTime);

        hasUninitializedPrototypes = *fetchFreshPrototypes.Execute ();
        hasPendingImmediatePrototypes = *fetchImmediatePrototypeAssemblies.Execute ();
        hasPendingWaitingPrototypes = *fetchWaitingPrototypeAssemblies.Execute ();
    }
}

void Assembler::StartFreshPrototypeAssembly () noexcept
{
    for (auto cursor = modifyFreshPrototypes.Execute (); auto *prototype = static_cast<PrototypeComponent *> (*cursor);)
    {
        auto prototypeAssemblyCursor = insertPrototypeAssembly.Execute ();
        auto *prototypeAssembly = static_cast<PrototypeAssemblyComponent *> (++prototypeAssemblyCursor);
        prototypeAssembly->objectId = prototype->objectId;

        prototypeAssembly->fixedAssemblyState = prototype->requestImmediateFixedAssembly ?
                                                    AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY :
                                                    AssemblyState::WAITING_FOR_ASSEMBLY;

        // Blocked until fixed assembly is done.
        prototypeAssembly->normalAssemblyState = AssemblyState::BLOCKED;
        prototype->assemblyStarted = true;
        ++cursor;
    }
}

void Assembler::ProcessImmediatePrototypes () noexcept
{
    for (auto cursor = modifyImmediatePrototypeAssemblies.Execute ();
         auto *assembly = static_cast<PrototypeAssemblyComponent *> (*cursor);)
    {
        switch (AssembleObject (assembly, 0u))
        {
        case AssemblyExecutionResult::DONE:
            ~cursor;
            break;
        case AssemblyExecutionResult::PARTIAL:
            ++cursor;
            break;
        case AssemblyExecutionResult::OUT_OF_TIME:
            // Cannot receive out of time result during immediate assembly.
            EMERGENCE_ASSERT (false);
            break;
        }
    }
}

void Assembler::ProcessWaitingPrototypes (std::uint64_t _executionStartTime) noexcept
{
    for (auto cursor = modifyWaitingPrototypeAssemblies.Execute ();
         auto *assembly = static_cast<PrototypeAssemblyComponent *> (*cursor);)
    {
        switch (AssembleObject (assembly, _executionStartTime))
        {
        case AssemblyExecutionResult::DONE:
            ~cursor;
            break;
        case AssemblyExecutionResult::PARTIAL:
            ++cursor;
            break;
        case AssemblyExecutionResult::OUT_OF_TIME:
            return;
        }
    }
}

AssemblyExecutionResult Assembler::AssembleObject (PrototypeAssemblyComponent *_assembly,
                                                   std::uint64_t _executionStartTime) noexcept
{
    Memory::UniqueString descriptorId;
    bool requestImmediateNormalAssembly = false;

    // We only fetch required data from PrototypeComponent and leave it alone, because more prototype
    // components can be inserted during object assembly (recursive prototypes are allowed).
    {
        auto prototypeCursor = fetchPrototypeById.Execute (&_assembly->objectId);
        const auto *prototype = static_cast<const PrototypeComponent *> (*prototypeCursor);

        if (!prototype)
        {
            EMERGENCE_LOG (ERROR, "Assembly: Unable to assemble object with id ", _assembly->objectId,
                           ", because it has no PrototypeComponent!");
            return AssemblyExecutionResult::DONE;
        }

        descriptorId = prototype->descriptorId;
        requestImmediateNormalAssembly = prototype->requestImmediateNormalAssembly;
    }

    Math::Transform3d rootObjectTransform3d;
    if (needRootObjectTransform3d)
    {
        auto transformCursor = fetchTransform3dById.Execute (&_assembly->objectId);
        if (const auto *transform = static_cast<const Transform3dComponent *> (*transformCursor))
        {
            rootObjectTransform3d = isFixed ? transform->GetLogicalWorldTransform (transformWorldAccessor) :
                                              transform->GetVisualWorldTransform (transformWorldAccessor);
        }
        else
        {
            EMERGENCE_LOG (ERROR, "Assembly: Unable to assemble object with id ", _assembly->objectId,
                           ", because it has no Transform3dComponent!");
            return AssemblyExecutionResult::DONE;
        }
    }

    auto descriptorCursor = fetchDescriptorById.Execute (&descriptorId);
    const auto *descriptor = static_cast<const AssemblyDescriptor *> (*descriptorCursor);

    if (!descriptor)
    {
        EMERGENCE_LOG (ERROR, "Assembly: Unable to find AssemblyDescriptor with id \"", descriptorId, "\"!");
        return AssemblyExecutionResult::DONE;
    }

    if (!_assembly->intermediateIdReplacement.empty ())
    {
        // Inherit intermediate id replacement from previous pass during other routine.
        EMERGENCE_ASSERT (_assembly->intermediateIdReplacement.size () == keyStates.size ());

        for (std::size_t index = 0u; index < keyStates.size (); ++index)
        {
            for (const auto &[from, to] : _assembly->intermediateIdReplacement[index])
            {
                keyStates[index].idReplacement.emplace (from, to);
            }
        }
    }

    auto saveIntermediateIdReplacement = [this, _assembly] ()
    {
        _assembly->intermediateIdReplacement.resize (
            keyStates.size (),
            Container::HashMap<UniqueId, UniqueId> {_assembly->intermediateIdReplacement.get_allocator ()});

        for (std::size_t index = 0u; index < keyStates.size (); ++index)
        {
            for (const auto &[from, to] : keyStates[index].idReplacement)
            {
                _assembly->intermediateIdReplacement[index].emplace (from, to);
            }
        }
    };

    auto clearIntermediateIdReplacement = [this] ()
    {
        for (auto &keyState : keyStates)
        {
            keyState.idReplacement.clear ();
        }
    };

    const bool immediate = (isFixed ? _assembly->fixedAssemblyState : _assembly->normalAssemblyState) ==
                           AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY;
    GetObjectIdKeyState ().idReplacement.emplace (ASSEMBLY_ROOT_OBJECT_ID, _assembly->objectId);
    std::size_t &index = (isFixed ? _assembly->fixedCurrentComponentIndex : _assembly->normalCurrentComponentIndex);

    while (index < descriptor->components.size () &&
           (immediate || Time::NanosecondsSinceStartup () - _executionStartTime < assemblyTimeLimit))
    {
        const StandardLayout::Patch &componentDescriptor = descriptor->components[index];
        ++index;
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
                EMERGENCE_ASSERT (needRootObjectTransform3d);
                auto *vector = static_cast<Math::Vector3f *> (vectorField.GetValue (component));
                *vector = Math::Rotate (*vector, rootObjectTransform3d.rotation);
            }
        }
        else
        {
            EMERGENCE_LOG (VERBOSE, "Skipping assembly of unknown type \"",
                           componentDescriptor.GetTypeMapping ().GetName (), "\"...");
        }
    }

    if (index < descriptor->components.size ())
    {
        saveIntermediateIdReplacement ();
        clearIntermediateIdReplacement ();
        return AssemblyExecutionResult::OUT_OF_TIME;
    }

    if (isFixed)
    {
        _assembly->fixedAssemblyState = AssemblyState::ASSEMBLED;
        _assembly->normalAssemblyState = requestImmediateNormalAssembly ? AssemblyState::IN_NEED_OF_IMMEDIATE_ASSEMBLY :
                                                                          AssemblyState::WAITING_FOR_ASSEMBLY;
        saveIntermediateIdReplacement ();
    }
    else
    {
        _assembly->normalAssemblyState = AssemblyState::ASSEMBLED;
    }

    (isFixed ? _assembly->fixedAssemblyState : _assembly->normalAssemblyState) = AssemblyState::ASSEMBLED;
    clearIntermediateIdReplacement ();

    auto eventCursor = insertFinishedEvent.Execute ();
    void *event = ++eventCursor;
    *static_cast<UniqueId *> (event) = _assembly->objectId;
    return isFixed ? AssemblyExecutionResult::PARTIAL : AssemblyExecutionResult::DONE;
}

UniqueId Assembler::ReplaceId (Assembler::KeyState &_keyState, UniqueId _id) noexcept
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

Assembler::KeyState &Assembler::GetObjectIdKeyState () noexcept
{
    return keyStates.back ();
}

Assembler::KeyState &Assembler::GetKeyState (UniqueId _index) noexcept
{
    if (_index == ASSEMBLY_OBJECT_ID_KEY_INDEX)
    {
        return GetObjectIdKeyState ();
    }

    EMERGENCE_ASSERT (_index < keyStates.size () - 1u);
    return keyStates[_index];
}

using namespace Memory::Literals;

static void AddCheckpoints (PipelineBuilder &_pipelineBuilder)
{
    _pipelineBuilder.AddCheckpoint (Assembly::Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Assembly::Checkpoint::FINISHED);
}

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder,
                       const CustomKeyVector &_allCustomKeys,
                       const TypeBindingVector &_fixedUpdateTypes,
                       std::uint64_t _maxAssemblyTimePerFrameNs) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, PrototypeComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Assembly::RemovePrototypesAssemblies"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupFixedEvent, PrototypeAssemblyComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Assembly");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask ("Assembly::FixedUpdate"_us)
        .SetExecutor<Assembler> (_allCustomKeys, _fixedUpdateTypes, true, _maxAssemblyTimePerFrameNs);
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const CustomKeyVector &_allCustomKeys,
                        const TypeBindingVector &_normalUpdateTypes,
                        std::uint64_t _maxAssemblyTimePerFrameNs) noexcept
{
    _pipelineBuilder.AddTask ("Assembly::RemovePrototypes"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, PrototypeComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Assembly::RemovePrototypeAssemblies"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, PrototypeAssemblyComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    // We don't care about fixed-to-normal transform removal events,
    // because in this case prototype will be removed through the special task in fixed update.

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Assembly");
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask ("Assembly::NormalUpdate"_us)
        .SetExecutor<Assembler> (_allCustomKeys, _normalUpdateTypes, false, _maxAssemblyTimePerFrameNs);
}
} // namespace Emergence::Celerity::Assembly
