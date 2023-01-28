#pragma once

#include <Celerity/Event/EventTrigger.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Pipeline.hpp>
#include <Celerity/Query/ModifySingletonQuery.hpp>

#include <Container/HashSet.hpp>
#include <Container/TypedOrderedPool.hpp>
#include <Container/Vector.hpp>

#include <Memory/OrderedPool.hpp>

#include <Warehouse/Registry.hpp>

namespace Emergence::Celerity
{
struct WorldSingleton;

// TODO: Note about performance.
//       Celerity is built on top of Warehouse and RecordCollection, and that services use standard layout mappings
//       extensively. It is convenient from code architecture perspective, but may result in performance problems.
//       There is a lot of mapping-related logic, that should be optimized, but nevertheless mapping usage creates
//       unavoidable performance barrier.
//       If all modules (both engine-level and game-level) are compiled from single source, then all used mapping data
//       is visible during compile time. We can use this fact for Chameleon-like approach: scan codebase and generate
//       optimized code for all storages, that supports all use cases by hard coding them. To make code scanning easier
//       we might use macro-driven approach like UnrealEngine. This code generation allows us to achieve hard code
//       performance without losing Celerity features, but it has one significant downside: we can no longer separate
//       game source and engine source, otherwise Chameleon will not be able to generate code for all use cases.

/// \brief Contains configuration for WorldView initialization.
struct WorldViewConfig final
{
    /// \brief Even if these types are not used by the tasks in view pipelines,
    ///        storages for them will be created in this view registry.
    /// \invariant For any path from root view to any child view, any type can not be mentioned in this
    ///            set more than once. So, if type is enforced in parent view, child views can not enforce it.
    Container::HashSet<StandardLayout::Mapping> enforcedTypes {Memory::Profiler::AllocationGroup::Top ()};
};

/// \brief Combination of World data partition that is visible to this view and pipelines that operate on this
///        partition. Provides way to split World into partitions that can be added or removed during World lifetime.
///
/// \par Visibility
/// \parblock
/// Views are designed around a concept of visibility: we need to provide a tool that allows to:
/// - Make some data inside the world invisible from other partitions.
/// - Have different storages for the same type of data, that can not be mutually visible.
///
/// Therefore, world views are organized as a tree. Root view only sees data in its own registry. Every child view
/// sees everything that its parent sees. In this case, children of the same parent may have independent data that
/// is invisible to the other children of the parent, which solves both requirements above.
/// \endparblock
///
/// \par Update
/// \parblock
/// Every view might have normal and fixed update pipelines, that operate in the context of this view.
/// When executing update, pipelines are firstly called for parent views and then for their child views
/// in the order of addition.
/// \endparblock
///
/// \par Events
/// \parblock
/// Event consumption is only allowed inside view that actually owns the event storage. This limitation was introduced
/// due to the fact that we need to validate event production and consumption so partial consumption never happens,
/// but this validation is impossible in multiview environment and partial consumption problems are very likely to
/// occur. Therefore, limiting event consumption to the owning view is the safest approach. Having said that, event
/// production is not limited to the owner view and is allowed to be done anywhere where event is visible.
/// \endparblock
///
/// \par Usage
/// \parblock
/// Initially World was designed as a single storage that cannot be partitioned in any way, but then need
/// to split World into partitions, that can be added or removed during game lifetime, has arisen.
///
/// There are two primary use cases for this feature:
///
/// - We need to share huge amounts of data between scenes, for example asset cache. We can travel from
///   one level to another and both levels might share lots of prefabs, models and other assets. In this
///   case, we definitely want to preserve these assets, but also we need to clean up all the runtime data
///   from previous level. Views provide a way to solve this problem: assets and other level-independent
///   data will be stored in the root view, while level-specific objects will be stored in child view
///   that can be easily removed and replaced by the child view for another level.
///
/// - In the editor, we need to be able to edit several resources, for example several levels, independently.
///   We cannot store all the opened documents in the one world as they will blend and may change each other.
///   But we cannot store them in separate worlds too, as their logic must share asset cache and document
///   model. We can solve this by storing asset cache, document model and other framework-level data in the
///   root view, while creating a separate child view for each document.
/// \endparblock
class WorldView final
{
public:
    WorldView (const WorldView &_other) = delete;

    WorldView (WorldView &&_other) = delete;

    /// \return Name of this view. Used for debug and logging purposes only.
    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    /// \brief Removes given pipeline from the world view and frees used memory.
    /// \invariant Pipeline belongs to this world view.
    void RemovePipeline (Pipeline *_pipeline) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (WorldView);

private:
    friend class EventRegistrar;
    friend class PipelineBuilder;
    friend class TaskConstructor;
    friend class World;
    friend class WorldTestingUtility;

    template <typename Type, typename... Arguments>
    friend Type ConstructInsideGroup (const Memory::Profiler::AllocationGroup &_group, Arguments... _arguments);

    struct EventSchemeInstance final
    {
        EventSchemeInstance (const Memory::Profiler::AllocationGroup &_rootAllocationGroup) noexcept;

        Container::TypedOrderedPool<TrivialEventTriggerInstanceRow> onAdd;
        Container::TypedOrderedPool<TrivialEventTriggerInstanceRow> onRemove;
        Container::TypedOrderedPool<OnChangeEventTriggerInstanceRow> onChange;
    };

    WorldView (World *_world, WorldView *_parent, Memory::UniqueString _name, const WorldViewConfig &_config) noexcept;

    ~WorldView () noexcept;

    Pipeline *AddPipeline (Memory::UniqueString _id, PipelineType _type, const Task::Collection &_collection) noexcept;

    void ExecuteNormalPipeline () noexcept;

    void ExecuteFixedPipeline () noexcept;

    WorldView &FindViewForType (const StandardLayout::Mapping &_type) noexcept;

    TrivialEventTriggerInstanceRow *RequestTrivialEventInstances (
        Container::TypedOrderedPool<TrivialEventTriggerInstanceRow> &_pool, const TrivialEventTriggerRow *_source);

    TrivialEventTriggerInstanceRow *RequestOnAddEventInstances (PipelineType _pipeline,
                                                                const TrivialEventTriggerRow *_source) noexcept;

    TrivialEventTriggerInstanceRow *RequestOnRemoveEventInstances (PipelineType _pipeline,
                                                                   const TrivialEventTriggerRow *_source) noexcept;

    OnChangeEventTriggerInstanceRow *RequestOnChangeEventInstances (PipelineType _pipeline,
                                                                    const ChangeTracker *_source) noexcept;

    World *world = nullptr;
    WorldView *parent = nullptr;
    Memory::UniqueString name;
    WorldViewConfig config;

    Warehouse::Registry localRegistry;
    Memory::OrderedPool pipelinePool;
    Pipeline *normalPipeline = nullptr;
    Pipeline *fixedPipeline = nullptr;

    Memory::Heap childrenHeap;
    Container::Vector<WorldView *> childrenViews;
    std::array<EventSchemeInstance, static_cast<std::size_t> (PipelineType::COUNT)> eventSchemeInstances;
};

/// \brief Contains basic configuration for WorldSingleton and TimeSingleton.
/// \details These values can be set through initialization pipeline, therefore
///          this structure is only a more convenient wa to do this task.
struct WorldConfiguration final
{
    /// \see TimeSingleton::targetFixedFrameDurationsS
    Container::InplaceVector<float, TimeSingleton::MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS {
        1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f};

    /// \brief Configuration for the root WorldView.
    WorldViewConfig rootViewConfig {};
};

/// \brief Represents whole game level (or world itself), works as conduit for data, events and pipelines.
class World final
{
public:
    World (Memory::UniqueString _name, const WorldConfiguration &_configuration = {}) noexcept;

    World (const World &_other) = delete;

    World (World &&_other) = delete;

    ~World () = default;

    /// \return Root view of this world.
    [[nodiscard]] const WorldView *GetRootView () const noexcept;

    /// \return Root view of this world.
    [[nodiscard]] WorldView *GetRootView () noexcept;

    /// \brief Constructs new view inside this world.
    /// \invariant _parent belongs to this world.
    WorldView *CreateView (WorldView *_parent, Memory::UniqueString _name, const WorldViewConfig &_config) noexcept;

    /// \brief Drops given view from this world.
    /// \invariant _view belongs to this world.
    void DropView (WorldView *_view) noexcept;

    /// \brief Executes normal update and fixed update if needed.
    /// \details Calculates time deltas and updates time, then executes normal pipeline and decides whether it is
    ///          needed to execute fixed pipeline.
    /// \invariant Should be called every frame if gameplay world is active. Correctly processes long absence of calls.
    void Update () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (World);

private:
    friend class EventRegistrar;
    friend class PipelineBuilder;
    friend class TaskConstructor;
    friend class WorldTestingUtility;

    struct CustomEventInfo final
    {
        StandardLayout::Mapping type;
        EventRoute route;
    };

    struct EventScheme final
    {
        EventScheme (const Memory::Profiler::AllocationGroup &_rootAllocationGroup) noexcept;

        Container::TypedOrderedPool<CustomEventInfo> custom;
        Container::TypedOrderedPool<TrivialEventTriggerRow> onAdd;
        Container::TypedOrderedPool<TrivialEventTriggerRow> onRemove;
        Container::TypedOrderedPool<OnChangeEventTrigger> onChange;
        Container::TypedOrderedPool<ChangeTracker> changeTrackers;
    };

    static void TimeUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    void FixedUpdate (TimeSingleton *_time, WorldSingleton *_world) noexcept;

    void EnsureViewIsOwned (WorldView *_view) noexcept;

    /// \details First field, because event scheme must be available during root world destruction.
    std::array<EventScheme, static_cast<std::size_t> (PipelineType::COUNT)> eventSchemes;
    WorldView rootView;

    Warehouse::ModifySingletonQuery modifyTime;
    Warehouse::ModifySingletonQuery modifyWorld;
};

/// \brief Contains useful functions for tests.
class WorldTestingUtility final
{
public:
    WorldTestingUtility () = delete;

    /// \brief Updates only normal time by given delta and runs normal update once.
    /// \warning Do not use if normal-fixed time integrity is required! Update world conventionally instead.
    static void RunNormalUpdateOnce (World &_world, uint64_t _timeDeltaNs) noexcept;

    /// \brief Updates only fixed time by one frame and runs fixed update once.
    /// \warning Do not use if normal-fixed time integrity is required! Update world conventionally instead.
    static void RunFixedUpdateOnce (World &_world) noexcept;

private:
    static std::pair<TimeSingleton *, WorldSingleton *> ExtractSingletons (World &_world) noexcept;
};
} // namespace Emergence::Celerity
