# Flow

Flow is a compact task registration library that provides user-friendly registration interface and exports registration
results as [TaskCollection](../../Private/TaskCollection/README.md). In order to make registration more convenient, Flow
provides some additional features:

- Dependency injection: task is able to make itself a dependency of other tasks. Using this technique game-level tasks,
  that do some additional game-specific work, can be easily placed between engine-level tasks.
- Checkpoints: user can declare logical milestones, called checkpoints, to avoid web-like dependency graphs. Both usual
  dependency declaration and dependency injection work with checkpoints. And they will not affect
  exported [TaskCollection](../../Private/TaskCollection/README.md): Flow erases them during export.
- Resource usage registration: tasks can declare read and write accesses to registered resources. This allows Flow to
  detect possible data races between tasks.
- Verification: if Flow exports non-empty [TaskCollection](../../Private/TaskCollection/README.md), than it's guaranteed
  that there is no cycles, missing dependencies and data races.
- Visualization: even if there are mistakes in registered tasks, whole pipeline with resources could be visualized
  as [VisualGraph](../VisualGraph/README.md). It's much easier to pinpoint issues visually.
