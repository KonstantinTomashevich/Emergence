# Emergence [WIP]  

[![Test](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml/badge.svg?branch=master&event=push)](https://github.com/KonstantinTomashevich/Emergence/actions/workflows/Test.yml)

Emergence is a flexible set of libraries for gameplay development, 
which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). 
Currently in development.

## Finished modules

### Services

- **Hashing**: allows to hash byte sequences.
- **MemoryPool**: paged memory allocator, inspired by **Boost.Pool**.
- **RecordCollection**: storage for records of the same type, that provides
  fast lookups for point, linear and volumetric representations of records.  
- **StandardLayoutMapping**: fast field-only reflection for 
  standard layout structures.
- **Testing**: hides selected testing framework under minimal set of 
  framework-agnostic macros.

### Libraries

- **Handling**: compact handle-based ownership implementation.
- **Pegasus**: provides storage and indexing for records of the same type.
  Optimized for small count of indices. Used to implement **RecordCollection**.

## Modules under development

### Services

- **Warehouse**: allows to store records of different types and effectively
  modify them through prepared queries using database-like API.

### Libraries

- **APICommon**: contains macros that declare common method and operator sets
  and implementation binding approaches. It's sort of always under development,
  because need for new shared API-related macro could arise during development 
  of new services.
- **Galleon**: database-like storage for records of different types, which is 
  built on top of **RecordCollection**. Used to implement **Warehouse**.
- **SyntaxSugar**: contains syntax sugar for other libraries 
  and service implementations. It's sort of always under development,
  because need for the new syntax sugar helpers could arise during 
  development of other modules.

## Planed modules

### Services

- **GraphExporter**: allows to create graph from runtime data and export it to 
  well known format, for example DOT. This service helps to visualize complex
  structures, that are created during game execution. For example, 
  task dependency graphs or data processing pipelines.
- **TaskOrganizer**: organizes tasks, that can use resources from 
  shared resources list and can depend on each other, into acyclic directed 
  graph, if it's possible.
- **TaskExecutor**: executes tasks, organized into acyclic directed graph. 
  Both sequential and parallel implementations are planned.
- **EventBasedSimulation**: gameplay programming library, that extends 
  `Entity-Component-System` pattern with `Event`'s. Planned to be implemented 
  on top of **Warehouse**, **TaskOrganizer** and **TaskExecutor** services,
  but will not expose their APIs.

### Libraries

- Base libraries, used to implement services, listed above.
- Wrapper for **EventBasedSimulation** service, that uses templates to 
  make **EventBasedSimulation** service more user friendly.
