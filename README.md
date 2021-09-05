# Emergence [WIP]

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development, which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). Under development.

## Finished modules

### Services

- **ExportGraph**: exports **VisualGraph** instances to well known formats, like DOT.
- **Hashing**: allows hashing byte sequences.
- **MemoryPool**: paged memory allocator, inspired by **Boost.Pool**.
- **RecordCollection**: storage for records of the same type, that provides fast lookups for point, linear and
  volumetric representations of records.
- **StandardLayoutMapping**: fast field-only reflection for standard layout structures.
- **Testing**: hides selected testing framework under minimal set of framework-agnostic macros.
- **Warehouse**: allows storing records of different types and effectively modifying them through prepared queries using
  database-like API.

### Libraries

- **Galleon**: database-like storage manager for records of different types, which is built on top of
  **RecordCollection**. Used to implement **Warehouse**.
- **Handling**: compact handle-based ownership implementation.
- **Pegasus**: provides storage and indexing for records of the same type. Optimized for small count of indices. Used to
  implement **RecordCollection**.
- **VisualGraph**: data structure for runtime state representation in a form of graph. Helps to visualize complex  
  structures that are created during execution. For example, task dependency graphs or data processing pipelines.
  Currently, there is no visual attributes except labels: they will be added when need arises.

## Modules under development

### Services

### Libraries

- **APICommon**: contains macros that declare common method and operator sets and implementation binding approaches.
  It's sort of always under development, because need for new shared API-related macro could arise during development of
  new services.

- **SyntaxSugar**: contains syntax sugar for other libraries and service implementations. It's sort of always under
  development, because need for the new syntax sugar helpers could arise during development of other modules.

## Planed modules

### Services

- **TaskOrganizer**: organizes tasks, that can use resources from shared resources list and can depend on each other,
  into acyclic directed graph, if it's possible.
- **TaskExecutor**: executes tasks, organized into acyclic directed graph. Both sequential and parallel implementations
  are planned.
- **EventBasedSimulation**: gameplay programming library, that extends
  `Entity-Component-System` pattern with `Event`'s. Planned to be implemented on top of **Warehouse**, **TaskOrganizer**
  and **TaskExecutor** services, but will not expose their APIs.

### Libraries

- Base libraries, used to implement services, listed above.
- Wrapper for **EventBasedSimulation** service, that uses templates to make **EventBasedSimulation** service more
  user-friendly.
