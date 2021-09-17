# Public Libraries

In some cases there is no sense to organize module into service:

- Module solves compact generic problem, like Handling.
- Module contains simple utility helpers, like SyntaxSugar.
- Module contains shared generic data structures, like VisualGraph.

These modules are organized as **public** libraries, that are aimed to be as useful for end user as services.
