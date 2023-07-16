# MemoryProfiler<sup>Abstract</sup>

Toolset for memory usage registration and observation:

- Provides runtime allocation grouping through `AllocationGroup`.
- Supports allocation group hierarchy, for example `World -> Components -> MyVeryLargeComponent`.
- Event-driven memory usage observation through `Capture` API.
- Integrated with [Memory](../Memory/README.md) abstract unit.
