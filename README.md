# Emergence [WIP]

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development that aims to become modular game engine in the future.
Currently under development.

## Architecture

Emergence architecture is built upon concept of units: relatively small modular libraries that provide singular feature
to the user and depend on one another in order to do it. There is 3 kinds of units:

- Interface units provide only headers.
- Concrete units provide headers and one concrete implementation for them.
- Abstract units provide headers with abstract API and several implementations for them as concrete units.

Every unit is either an object or interface library, therefore it is possible to explicitly control what goes into
which output artefact (shared library or executable).

This architecture aims to:

- Make link time polymorphism easy to use through abstract units.
- Make modularization straightforward, easy to define and implement.
- Make it easy to separate modules into several shared libraries.

## Units

- [APICommon<sup>Interface</sup>](./Unit/APICommon): contains macros for declaration of common interfaces.
- [Assert<sup>Abstract</sup>](./Unit/Assert): provides assertion mechanism with optional fallback to interactive mode.
- [AssertSDL3<sup>Concrete</sup>](./Unit/AssertSDL3): implementation of abstract unit above that uses
  [SDL3](https://github.com/libsdl-org/SDL) for interactive mode implementation.
- [Celerity<sup>Concrete</sup>](./Unit/Celerity): combines [Warehouse](./Unit/Warehouse/README.md),
  [Flow](./Unit/Flow/README.md) and [TaskExecutor](./Unit/TaskExecutor/README.md) into ECS-like framework for gameplay
  world logic implementation.
- [CelerityAssembly<sup>Concrete</sup>](./Unit/CelerityAssembly/README.md): pattern implementation that makes
  it easy to instantiate objects during gameplay.
- [CelerityAsset<sup>Concrete</sup>](./Unit/CelerityAsset/README.md): implements asset cache behaviour with automatic
  asset management through reflection-based asset usage scanning.
- [CelerityInput<sup>Concrete</sup>](./Unit/CelerityInput/README.md): implements input abstraction layer for game-level
  input.
- [CelerityLocale<sup>Concrete</sup>](./Unit/CelerityLocale/README.md): simplistic localization implementation.
- [CelerityPhysics2d<sup>Abstract</sup>](./Unit/CelerityPhysics2d/README.md): provides 2d physics simulation.
- [CelerityPhysics2dBox2d<sup>Concrete</sup>](./Unit/CelerityPhysics2dBox2d/README.md): implementation of abstract unit
  above that uses [Box2d](https://box2d.org/) as backend.
- [CelerityPhysics2dCommon<sup>Concrete</sup>](./Unit/CelerityPhysics2dCommon/README.md): common logic for
  implementations of abstract unit above.
- [CelerityPhysics2dDebugDraw<sup>Concrete</sup>](./Unit/CelerityPhysics2dDebugDraw/README.md): provides debug draw for
  [CelerityPhysics2d](./Unit/CelerityPhysics2d/README.md) on top of
  [CelerityRender2d](./Unit/CelerityRender2d/README.md).
- [CelerityPhysics3d<sup>Abstract</sup>](./Unit/CelerityPhysics3d/README.md): provides 3d physics simulation.
- [CelerityPhysics3dCommon<sup>Concrete</sup>](./Unit/CelerityPhysics3dCommon/README.md): common logic for
  implementations of abstract unit above.
- [CelerityPhysics3dPhysX<sup>Concrete</sup>](./Unit/CelerityPhysics3dPhysX/README.md): implementation of abstract unit
  above that uses [PhysX](https://github.com/NVIDIAGameWorks/PhysX) as backend.
- [CelerityRender2d<sup>Concrete</sup>](./Unit/CelerityRender2d/README.md): simplistic 2d rendering implementation.
- [CelerityRenderFoundation](./Unit/CelerityRenderFoundation/README.md): contains common features and
  routine for render extensions making it possible to glue them together.
- [CelerityResourceConfig<sup>Concrete</sup>](./Unit/CelerityResourceConfig/README.md): provides support for game config
  loading using message driven API.
- [CelerityResourceObject<sup>Concrete</sup>](./Unit/CelerityResourceObject/README.md): integrates
  [CelerityAssembly](./Unit/CelerityAssembly/README.md) with
  [ResourceObject](./Unit/ResourceObject/README.md) using message driven API.
- [CelerityTransform](./Unit/CelerityTransform/README.md): transform node hierarchy with logical and
  visual transform separation and interpolation-based visual transform sync.
- [CelerityUI<sup>Abstract</sup>](./Unit/CelerityUI/README.md): provides simplistic UI features.
- [CelerityUICommon<sup>Concrete</sup>](./Unit/CelerityUICommon/README.md): common logic for
  implementations of abstract unit above.
- [CelerityUIImGUI<sup>Concrete</sup>](./Unit/CelerityUIImGUI/README.md): implementation of abstract unit
  above that uses [ImGUI](https://github.com/ocornut/imgui) as backend.
- [CommandLine<sup>Concrete</sup>](./Unit/CommandLine/README.md): provides utilities for command line applications.
- [Container<sup>Concrete</sup>](./Unit/Container/README.md): template container proxies and implementations.
- [CPUProfiler<sup>Abstract</sup>](./Unit/CPUProfiler/README.md): provides minimalistic interface for setting up CPU
  usage profiling.
- [CPUProfilerNone<sup>Concrete</sup>](./Unit/CPUProfilerNone/README.md): implementation of abstract unit above that
  disables CPU profiling.
- [CPUProfilerTracy<sup>Concrete</sup>](./Unit/CPUProfilerTracy/README.md): implementation of abstract unit above that
  uses [Tracy](https://github.com/wolfpld/tracy) as backend library.
- [ExportGraph<sup>Abstract</sup>](./Unit/ExportGraph/README.md): exports
  [VisualGraph<sup>Concrete</sup>](./Unit/VisualGraph/README.md) instances to well known formats.
- [ExportGraphDOT<sup>Concrete</sup>](./Unit/ExportGraphDOT/README.md): implementation of abstract unit above that
  exports graphs to [DOT](https://graphviz.org/doc/info/lang.html) format.
- [Flow<sup>Concrete</sup>](./Unit/Flow/README.md): compact user-friendly task registration library. It verifies that
  there is no circular dependencies, no missing dependencies and no data races between registered tasks.
- [Galleon<sup>Concrete</sup>](./Unit/Galleon/README.md): original implementation
  of [Warehouse](./Unit/Warehouse/README.md) that is historically provided as separate unit.
- [Handling<sup>Concrete</sup>](./Unit/Handling/README.md): compact handle-based ownership implementation.
- [Hashing<sup>Abstract</sup>](./Unit/Hashing/README.md): provides byte sequence hashing.
- [HashingXXHash<sup>Concrete</sup>](./Unit/HashingXXHash/README.md): implementation of abstract unit above that
  uses [XXHash](https://github.com/Cyan4973/xxHash) as backend.
- [JobDispatcher<sup>Abstract</sup>](./Unit/JobDispatcher/README.md): provides simple interface for scheduling job
  execution on multiple threads.
- [JobDispatcherOriginal<sup>Concrete</sup>](./Unit/JobDispatcherOriginal/README.md): original implementation of
  abstract unit above.
- [Log<sup>Abstract</sup>](./Unit/Log/README.md): provides simple logging interface.
- [LogSPDLog<sup>Concrete</sup>](./Unit/LogSPDLog/README.md): implementation of abstract unit above that
  uses [spdlog](https://github.com/gabime/spdlog) as backend.
- [Math<sup>Concrete</sup>](./Unit/Math/README.md): math classes and functions for game programming.
- [Memory<sup>Abstract</sup>](./Unit/Memory/README.md): set of mechanisms for rational memory usage.
- [MemoryOriginal<sup>Concrete</sup>](./Unit/MemoryOriginal/README.md): original implementation of abstract unit above.
- [MemoryProfiler<sup>Abstract</sup>](./Unit/MemoryProfiler/README.md): provides mechanism for memory usage registration
  and observation, integrated with [Memory](./Unit/Memory/README.md) unit.
- [MemoryProfilerNone<sup>Concrete</sup>](./Unit/MemoryProfilerNone/README.md): implementation of abstract unit above
  that disables memory profiling.
- [MemoryProfilerOriginal<sup>Concrete</sup>](./Unit/MemoryProfilerOriginal/README.md): original implementation of
  abstract unit above.
- [MemoryRecording<sup>Concrete</sup>](./Unit/MemoryRecording/README.md): toolset for memory usage recording and
  replaying.
- [Pegasus<sup>Concrete</sup>](./Unit/Pegasus/README.md): original implementation
  of [RecordCollection](./Unit/RecordCollection/README.md) that is historically provided as separate unit.
- [RecordCollection<sup>Abstract</sup>](./Unit/RecordCollection/README.md): storage for records of the same type, that
  provides fast lookups for point, linear, signal and volumetric representations of records.
- [RecordCollectionPegasus<sup>Concrete</sup>](./Unit/RecordCollectionPegasus/README.md): implementation of abstract
  unit above that uses [Pegasus](./Unit/Pegasus/README.md) as backend.
- [RecordCollectionVisualization<sup>Concrete</sup>](./Unit/RecordCollectionVisualization/README.md): provides functions
  for [RecordCollection](./Unit/RecordCollection/README.md) runtime structure visualization.
- [RenderBackend<sup>Abstract</sup>](./Unit/RenderBackend/README.md): provides low level render features like programs,
  uniforms, buffers and so on.
- [RenderBackendBGFX<sup>Concrete</sup>](./Unit/RenderBackendBGFX/README.md): implementation of abstract unit above that
  uses [BGFX](https://github.com/bkaradzic/bgfx) library as backend.
- [ResourceCooking<sup>Concrete</sup>](./Unit/ResourceCooking/README.md): provides foundation and stores common logic
  for resource cooking tools.
- [ResourceObject<sup>Concrete</sup>](./Unit/ResourceObject/README.md): provides format for storing game objects (like
  units, buildings, etc.) in YAML or binary with single inheritance support.
- [ResourceProvider<sup>Abstract</sup>](./Unit/ResourceProvider/README.md): provides out-of-the-box resource discovery
  solution that supports multiple resource sources.
- [ResourceProviderHelpers<sup>Interface</sup>](./Unit/ResourceProviderHelpers/README.md): inline helper functions that
  make usage of the service above easier.
- [ResourceProviderOriginal<sup>Concrete</sup>](./Unit/ResourceProviderOriginal/README.md): original implementation of
  abstract unit above.
- [Serialization<sup>Concrete</sup>](./Unit/Serialization/README.md): toolset for serializing and deserializing data
  from binary and yaml formats using [StandardLayoutMapping](./Unit/StandardLayoutMapping/README.md).
- [StandardLayoutMapping<sup>Abstract</sup>](./Unit/StandardLayoutMapping/README.md): fast field-only reflection for
  standard layout structures.
- [StandardLayoutMappingOriginal<sup>Concrete</sup>](./Unit/StandardLayoutMappingOriginal/README.md): original
  implementation of abstract unit above.
- [StandardLayoutMappingVisualization<sup>Concrete</sup>](./Unit/StandardLayoutMappingVisualization/README.md): provides
  functions for [StandardLayoutMapping](./Unit/StandardLayoutMapping/README.md) runtime structure visualization.
- [TaskCollection<sup>Concrete</sup>](./Unit/TaskCollection/README.md): intermediate format for storing tasks that may
  depend on each other.
- [TaskExecutor<sup>Abstract</sup>](./Unit/TaskExecutor/README.md): provides execution interface for tasks, stored
  in [TaskCollection](./Unit/TaskCollection/README.md) format.
- [TaskExecutorParallel<sup>Concrete</sup>](./Unit/TaskExecutorParallel/README.md): parallel implementation of abstract
  unit above.
- [TaskExecutorSequential<sup>Concrete</sup>](./Unit/TaskExecutorSequential/README.md): sequential implementation of
  abstract unit above.
- [Testing<sup>Interface</sup>](./Unit/Testing/README.md): hides selected testing framework under minimal set of
  framework-agnostic macros.
- [Threading<sup>Concrete</sup>](./Unit/Threading/README.md): provides utilities for multithreading code like atomic
  lock guard.
- [Time<sup>Concrete</sup>](./Unit/Time/README.md): provides utilities for code that needs to work with time.
- [Traits<sup>Interface</sup>](./Unit/Traits/README.md): provides various simple utilities for working with templates
  and unions.
- [VirtualFileSystem<sup>Abstract</sup>](./Unit/VirtualFileSystem/README.md): provides file system abstraction layer
  that supports mounting of real file system directories and read-only binary packages (for optimized reading).
- [VirtualFileSystemHelpers<sup>Interface</sup>](./Unit/VirtualFileSystemHelpers/README.md): inline helper functions
  that make usage of the service above easier.
- [VirtualFileSystemOriginal<sup>Concrete</sup>](./Unit/VirtualFileSystemOriginal/README.md): original implementation of
  abstract unit above.
- [VisualGraph<sup>Concrete</sup>](./Unit/VisualGraph/README.md): data structure for runtime state representation in a
  form of graph. Helps to visualize complex structures that are created during execution. For example, task dependency
  graphs or data processing pipelines.
- [Warehouse<sup>Abstract</sup>](./Unit/Warehouse/README.md): provides database-like API for storing objects and
  accessing them through prepared queries.
- [WarehouseGalleon<sup>Concrete</sup>](./Unit/WarehouseGalleon/README.md): implements abstract unit above
  using [Galleon](./Unit/Galleon/README.md) as backend.
- [WarehouseVisualization<sup>Concrete</sup>](./Unit/WarehouseVisualization/README.md): provides functions
  for [Warehouse](./Unit/Warehouse/README.md) runtime structure visualization.

## Executables

- [MemoryRecordingClient](./Executable/MemoryRecordingClient/README.md):
  tool for viewing [MemoryRecording](./Unit/MemoryRecording/README.md) serialized tracks.
- [Platformer2dDemo](./Executable/Platformer2dDemo/README.md): simple platformer game in early stage of development,
  used to test new **Emergence** features.
