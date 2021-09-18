# Public Libraries

In some cases there is no sense to organize module as service:

- Module solves compact generic problem, like Handling.
- Module contains simple utility helpers, like SyntaxSugar.
- Module contains shared generic data structures, like VisualGraph.

These modules are organized as **public** libraries, that are aimed to be as useful for the end user as services.
