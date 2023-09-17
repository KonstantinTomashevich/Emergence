# Emergence

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

## Features

### Core

- Assertions with interactive mode in [Assert unit](./Unit/Assert/README.md).
- Interface for CPU profiling setup in [CPUProfiler unit](./Unit/CPUProfiler/README.md).
- Interface for byte sequence hashing in [Hashing unit](./Unit/Hashing/README.md).
- Intermediate layer for input registration in [InputStorage unit](./Unit/InputStorage/README.md).
- Lightweight job dispatch implementation in [JobDispatcher unit](./Unit/JobDispatcher/README.md).
- Logging with multiple sinks interface in [Log unit](./Unit/Log/README.md).
- Math structures for game development in [Math unit](./Unit/Math/README.md).
- Memory management structures in [Memory unit](./Unit/Memory/README.md).
- Memory usage profiling implementation in [MemoryProfiler unit](./Unit/MemoryProfiler/README.md).
- High level memory usage recording logic in [MemoryRecording unit](./Unit/MemoryRecording/README.md).
- Low level render abstraction layer in [RenderBackend unit](./Unit/RenderBackend/README.md).
- Reflection-driven serialization in [Serialization unit](./Unit/Serialization/README.md).
- Fast field-only reflection in [StandardLayoutMapping unit](./Unit/StandardLayoutMapping/README.md).
- Arbitrary task graph execution implementation in [TaskExecutor unit](./Unit/TaskExecutor/README.md) with task graph
  format in [TaskCollection unit](./Unit/TaskCollection/README.md).
- Testing backend abstraction layer in [Testing unit](./Unit/Testing/README.md).
- File system virtualization with packaging support in [VirtualFileSystem unit](./Unit/VirtualFileSystem/README.md).

### Resource management

- Simplistic routine for resource cooking implementation in [ResourceCooking unit](./Unit/ResourceCooking/README.md).
- Prefab-like resources with inheritance implementation in [ResourceObject unit](./Unit/ResourceObject/README.md).
- Resource scanning and loading layer in [ResourceProvider unit](./Unit/ResourceProvider/README.md).

### Data management and processing

- Task graph construction with race condition validation in [Flow unit](./Unit/Flow/README.md).
- Indexed data storage implementation in [RecordCollection unit](./Unit/RecordCollection/README.md) with visualization
  in [RecordCollectionVisualization unit](./Unit/RecordCollectionVisualization/README.md).
- Multi-type data storage with prepared queries support in [Warehouse unit](./Unit/Warehouse/README.md).

### Celerity

[Celerity](./Unit/Celerity/README.md) is an ECS-like game world architecture solution. It provides interface for
building modular worlds, that consist of multiple logical parts. For example, tools like unified editor might be
modelled as a world with multiple independent sub worlds. It has lots of extensions:

- `Assembly` pattern implementation in [CelerityAssemblyLogic](./Unit/CelerityAssemblyLogic/README.md)
  and [CelerityAssemblyModel](./Unit/CelerityAssemblyModel/README.md) units.
- Asset management routine implementation in [CelerityAssetLogic](./Unit/CelerityAssetLogic/README.md)
  and [CelerityAssetModel](./Unit/CelerityAssetModel/README.md) units.
- Input abstraction layer implementation in [CelerityInputLogic](./Unit/CelerityInputLogic/README.md)
  and [CelerityInputModel](./Unit/CelerityInputModel/README.md) units.
- Localization implementation in [CelerityLocaleLogic](./Unit/CelerityLocaleLogic/README.md)
  and [CelerityLocaleModel](./Unit/CelerityLocaleModel/README.md) units.
- Physics 2d simulation in [CelerityPhysics2dLogic](./Unit/CelerityPhysics2dLogic/README.md)
  and [CelerityPhysics2dModel](./Unit/CelerityPhysics2dModel/README.md) units with optional debug draw in
  [CelerityPhysics2dLogicDebugDraw](./Unit/CelerityPhysics2dLogic/README.md)
  and [CelerityPhysics2dModelDebugDraw](./Unit/CelerityPhysics2dModel/README.md) units.
- Physics 3d simulation in [CelerityPhysics3dLogic](./Unit/CelerityPhysics3dLogic/README.md)
  and [CelerityPhysics3dModel](./Unit/CelerityPhysics3dModel/README.md) units.
- Simplistic 2d rendering implementation in [CelerityRender2dLogic](./Unit/CelerityRender2dLogic/README.md)
  and [CelerityRender2dModel](./Unit/CelerityRender2dModel/README.md) units.
- Foundation toolkit for rendering implementation
  in [CelerityRenderFoundationLogic](./Unit/CelerityRenderFoundationLogic/README.md)
  and [CelerityRenderFoundationModel](./Unit/CelerityRenderFoundationModel/README.md) units.
- Configuration resources support in [CelerityResourceConfigLogic](./Unit/CelerityResourceConfigLogic/README.md)
  and [CelerityResourceConfigModel](./Unit/CelerityResourceConfigModel/README.md) units.
- Prefab-like resources support in [CelerityResourceObjectLogic](./Unit/CelerityResourceObjectLogic/README.md)
  and [CelerityResourceObjectModel](./Unit/CelerityResourceObjectModel/README.md) units.
- Transform hierarchy with logical/visual split implementation
  in [CelerityTransformLogic](./Unit/CelerityTransformLogic/README.md)
  and [CelerityTransformModel](./Unit/CelerityTransformModel/README.md) units.
- Simplistic UI implementation in [CelerityUILogic](./Unit/CelerityUILogic/README.md)
  and [CelerityUIModel](./Unit/CelerityUIModel/README.md) units.

### Other

- Utilities for command like tools in [CommandLine unit](./Unit/CommandLine/README.md).
- Common containers in [Container unit](./Unit/Container/README.md).
- Intermediate graph format in [VisualGraph unit](./Unit/VisualGraph/README.md) with export to common formats in
  [ExportGraph unit](./Unit/ExportGraph/README.md).
- Lightweight implementation of object handles in [Handling unit](./Unit/Handling/README.md).
- Utilities for multithreading in [Threading unit](./Unit/Threading/README.md).
- Utilities for working with time in [Time unit](./Unit/Time/README.md).
- Utilities for working with templates and unions in [Traits unit](./Unit/Traits/README.md).

## Executables

- [MemoryRecordingClient](./Executable/MemoryRecordingClient/README.md):
  tool for viewing [MemoryRecording](./Unit/MemoryRecording/README.md) serialized tracks.
- [Platformer2dDemo](./Executable/Platformer2dDemo/README.md): simple platformer game in early stage of development,
  used to test new **Emergence** features.
