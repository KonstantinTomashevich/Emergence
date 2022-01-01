# Emergence [WIP]

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development, which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). Under development.

## Finished modules

### Services

- [ExportGraph](./Service/ExportGraph/README.md): exports [VisualGraph](./Library/Public/VisualGraph/README.md)
  instances to well known formats, like DOT.
- [Hashing](./Service/Hashing/README.md): provides byte sequence hashing.
- [Log](./Service/Log/README.md): provides simple logging interface.
- [Memory](./Service/Memory/README.md): set of mechanisms for rational memory usage.
- [RecordCollection](./Service/RecordCollection/README.md): storage for records of the same type, that provides fast
  lookups for point, linear and volumetric representations of records.
- [StandardLayoutMapping](./Service/StandardLayoutMapping/README.md): fast field-only reflection for standard layout
  structures.
- [TaskExecutor](./Service/TaskExecutor/README.md): provides execution interface for tasks, stored
  in [TaskCollection](./Library/Private/TaskCollection/README.md) format.
- [Testing](./Service/Testing/README.md): hides selected testing framework under minimal set of framework-agnostic
  macros.
- [Warehouse](./Service/Warehouse/README.md): provides database-like API for storing objects and accessing them through
  prepared queries.

### Libraries

- [Container](./Library/Public/Container/README.md): template container proxies and implementations.
- [Flow](./Library/Public/Flow/README.md): compact user-friendly task registration library. It verifies that there is no
  circular dependencies, no missing dependencies and no data races between registered tasks.
- [Handling](./Library/Public/Handling/README.md): compact handle-based ownership implementation.
- [SyntaxSugar](./Library/Public/SyntaxSugar/README.md): contains syntax sugar for other libraries and service
  implementations.
- [VisualGraph](./Library/Public/VisualGraph/README.md): data structure for runtime state representation in a form of
  graph. Helps to visualize complex structures that are created during execution. For example, task dependency graphs or
  data processing pipelines.

## Planed modules

### Services

### Libraries

- **Celerity**: combines [Warehouse](./Service/Warehouse/README.md), **TaskOrganizer** and **TaskExecutor** into
  framework for gameplay world logic implementation.
