# Emergence [WIP]

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development, which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). Under development.

## Services

- [Assert](./Service/Assert/README.md): provides assertion mechanism with optional interactive mode.
- [ExportGraph](./Service/ExportGraph/README.md): exports [VisualGraph](./Library/Public/VisualGraph/README.md)
  instances to well known formats, like DOT.
- [Hashing](./Service/Hashing/README.md): provides byte sequence hashing.
- [JobDispatcher](./Service/JobDispatcher/README.md): provides simple interface for scheduling job execution
  on multiple threads.
- [Log](./Service/Log/README.md): provides simple logging interface.
- [Memory](./Service/Memory/README.md): set of mechanisms for rational memory usage.
- [MemoryProfiler](./Service/MemoryProfiler/README.md): provides mechanism for memory usage registration and
  observation, integrated with `Memory` service.
- [RecordCollection](./Service/RecordCollection/README.md): storage for records of the same type, that provides fast
  lookups for point, linear, signal and volumetric representations of records.
- [RenderBackend](./Service/RenderBackend/README.md): provides low level render features like programs, uniforms, 
  buffers and so on.
- [ResourceProvider](./Service/ResourceProvider/README.md): Provides out-of-the-box resource discovery solution 
  that supports multiple resource sources.
- [StandardLayoutMapping](./Service/StandardLayoutMapping/README.md): fast field-only reflection for standard layout
  structures.
- [TaskExecutor](./Service/TaskExecutor/README.md): provides execution interface for tasks, stored
  in [TaskCollection](./Library/Private/TaskCollection/README.md) format.
- [Testing](./Service/Testing/README.md): hides selected testing framework under minimal set of framework-agnostic
  macros.
- [Warehouse](./Service/Warehouse/README.md): provides database-like API for storing objects and accessing them through
  prepared queries.

### Celerity extensions

These services are built on top of [Celerity](./Library/Public/Celerity/README.md) library and extend its features:

- [Physics2d](./Service/Celerity/Extension/Physics2d/README.md): provides 2d physics simulation.
- [Physics3d](./Service/Celerity/Extension/Physics3d/README.md): provides 3d physics simulation.
- [UI](./Service/Celerity/Extension/UI/README.md): provides simplistic UI features.

## Libraries

- [Celerity](./Library/Public/Celerity/README.md): combines [Warehouse](./Service/Warehouse/README.md),
  [Flow](./Library/Public/Flow/README.md) and [TaskExecutor](./Service/TaskExecutor/README.md) into framework
  for gameplay world logic implementation.
- [Container](./Library/Public/Container/README.md): template container proxies and implementations.
- [Flow](./Library/Public/Flow/README.md): compact user-friendly task registration library. It verifies that there is no
  circular dependencies, no missing dependencies and no data races between registered tasks.
- [Handling](./Library/Public/Handling/README.md): compact handle-based ownership implementation.
- [Math](./Library/Public/Math/README.md): math classes and functions for game programming.
- [MemoryRecording](./Library/Public/MemoryRecording/README.md): toolset for memory usage recording and replaying.
- [ResourceObject](Library/Public/ResourceObject/README.md): provides format for storing game objects (like units,
  buildings, etc) in YAML or binary with single inheritance support.
- [Serialization](./Library/Public/Serialization/README.md): toolset for serializing and deserializing data from 
  binary and yaml formats using [StandardLayoutMapping](./Service/StandardLayoutMapping/README.md).
- [SyntaxSugar](./Library/Public/SyntaxSugar/README.md): contains syntax sugar for other libraries and service
  implementations.
- [VisualGraph](./Library/Public/VisualGraph/README.md): data structure for runtime state representation in a form of
  graph. Helps to visualize complex structures that are created during execution. For example, task dependency graphs or
  data processing pipelines.

### Celerity extensions

These libraries are built on top of [Celerity](./Library/Public/Celerity/README.md) library and extend its features:

- [Assembly](./Library/Public/Celerity/Extension/Assembly/README.md): `Assembly` pattern implementation that makes
  it easy to instantiate objects during gameplay.
- [Asset](./Library/Public/Celerity/Extension/Asset/README.md): implements asset cache behaviour with automatic asset
  management through reflection-based asset usage scanning.
- [Input](./Library/Public/Celerity/Extension/Input/README.md): implements input abstraction layer for game-level input.
- [Locale](./Library/Public/Celerity/Extension/Locale/README.md): simplistic localization implementation.
- [Physics2dDebugDraw](./Library/Public/Celerity/Extension/Physics2dDebugDraw/README.md): provides debug draw for
  [Physics2d](./Service/Celerity/Extension/Physics2d/README.md) on top of
  [Render2d](./Library/Public/Celerity/Extension/Render2d/README.md).
- [Render2d](./Library/Public/Celerity/Extension/Render2d/README.md): simplistic 2d rendering implementation.
- [RenderFoundation](./Library/Public/Celerity/Extension/RenderFoundation/README.md): contains common features and 
  routine for render extensions making it possible to glue them together.
- [ResourceConfig](./Library/Public/Celerity/Extension/ResourceConfig/README.md): provides support for game config 
  loading using message driven API.
- [ResourceObject](./Library/Public/Celerity/Extension/ResourceObject/README.md): integrates
  [Assembly](./Library/Public/Celerity/Extension/Assembly/README.md) with
  [ResourceObject](Library/Public/ResourceObject/README.md) using message driven API.
- [Transform](./Library/Public/Celerity/Extension/Transform/README.md): transform node hierarchy with logical and 
  visual transform separation and interpolation-based visual transform sync.

## Executables

- [MemoryRecordingClient](./Executable/MemoryRecordingClient/README.md):
  tool for viewing [MemoryRecording](./Library/Public/MemoryRecording/README.md) serialized tracks.
- [Platformer2dDemo](./Executable/Platformer2dDemo/README.md): simple platformer game in early stage of development,
  used to test new **Emergence** features.
- [SpaceShooterDemo](./Executable/SpaceShooterDemo/README.md): simple shooter game, that illustrates usage of
  **Emergence** features.
