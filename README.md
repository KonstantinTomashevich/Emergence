# Emergence [WIP]

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development, which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). Under development.

## Finished modules

### Services

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
- [StandardLayoutMapping](./Service/StandardLayoutMapping/README.md): fast field-only reflection for standard layout
  structures.
- [TaskExecutor](./Service/TaskExecutor/README.md): provides execution interface for tasks, stored
  in [TaskCollection](./Library/Private/TaskCollection/README.md) format.
- [Testing](./Service/Testing/README.md): hides selected testing framework under minimal set of framework-agnostic
  macros.
- [Warehouse](./Service/Warehouse/README.md): provides database-like API for storing objects and accessing them through
  prepared queries.

### Libraries

- [Celerity](./Library/Public/Celerity/README.md): combines [Warehouse](./Service/Warehouse/README.md),
  [Flow](./Library/Public/Flow/README.md) and [TaskExecutor](./Service/TaskExecutor/README.md) into framework
  for gameplay world logic implementation. Has transform hierarchy and physics engine extensions.
- [Container](./Library/Public/Container/README.md): template container proxies and implementations.
- [Flow](./Library/Public/Flow/README.md): compact user-friendly task registration library. It verifies that there is no
  circular dependencies, no missing dependencies and no data races between registered tasks.
- [Handling](./Library/Public/Handling/README.md): compact handle-based ownership implementation.
- [Math](./Library/Public/Math/README.md): math classes and functions for game programming.
- [MemoryRecording](./Library/Public/MemoryRecording/README.md): toolset for memory usage recording and replaying.
- [SyntaxSugar](./Library/Public/SyntaxSugar/README.md): contains syntax sugar for other libraries and service
  implementations.
- [VisualGraph](./Library/Public/VisualGraph/README.md): data structure for runtime state representation in a form of
  graph. Helps to visualize complex structures that are created during execution. For example, task dependency graphs or
  data processing pipelines.

### Executables

- [MemoryRecordingClient](./Executable/MemoryRecordingClient/README.md):
  tool for viewing [MemoryRecording](./Library/Public/MemoryRecording/README.md) serialized tracks.
- [SpaceShooterDemo](./Executable/SpaceShooterDemo/README.md): simple shooter game, that illustrates usage of
  **Emergence** features.
