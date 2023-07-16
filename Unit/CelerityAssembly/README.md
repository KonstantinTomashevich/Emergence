# CelerityAssembly<sup>Concrete</sup>

Implements `Assembly` pattern: fills objects with `PrototypeComponent` with components and sub-objects
from specified assembly descriptor. This pattern provides universal and flexible way to spawn complex
objects that can be altered by non-programmers through assets. Descriptors are built on top of 
[StandardLayoutMapping](../StandardLayoutMapping/README.md) patches and therefore  can be both loaded 
from resources and created during runtime.
