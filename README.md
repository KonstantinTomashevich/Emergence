# Emergence [WIP]

Emergence is a flexible set of libraries for gameplay development, 
which modular architecture is inspired by
[Sober CMake Framework](https://github.com/KonstantinTomashevich/Sober). 
Currently in development.

## Finished modules

### Services

- **StandardLayoutMapping**: fast field-only reflection 
  for standard layout structures.
- **MemoryPool**: paged memory allocator, inspired by **Boost.Pool**.

### Libraries

- **Handling**: compact handle-based ownership implementation.

## Modules under development

### Services

- **RecordCollection**: storage for records of the same type, that provides 
  fast lookups for point, linear and volumetric representations of records.

### Libraries

- **SyntaxSugar**: contains syntax sugar for other libraries 
   and service implementations.
- **Pegasus**: base library for one of the **RecordCollection** implementations.
  Optimized for collections with small count of representations.

## Planed modules

### Services

- **GraphExporter**: allows to create graph from runtime data and export it to 
  well known format, for example DOT. This service helps to visualize complex
  structures, that are created during game execution. For example, 
  task dependency graphs or data processing pipelines.
- **Warehouse**: allows to store records of different types and effectively 
  modify them through prepared queries using database-like API. Planned to be 
  implemented on top of **RecordCollection** service, but will not expose 
  **RecordCollection** API.
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