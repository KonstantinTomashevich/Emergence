### ResourceObject<sup>Concrete</sup>

This library provides format for storing game objects (like units, buildings, etc) in YAML or binary. It is designed
to be used with [CelerityAssembly](../CelerityAssembly/README.md), but can be used as separate library too.

Objects consist of parts, like entities in ECS, and each part is defined by
[StandardLayoutMapping](../StandardLayoutMapping/README.md) patch, much like
[CelerityAssembly](../CelerityAssembly/README.md) defines components in `AssemblyDescriptor`. In contrast to 
`AssemblyDescriptor`s, objects support single inheritance: for example, you can create base object that suits any unit, 
then extend it with components in child objects that are created for every unit. And then, you can create child classes
with unit skins too! `ResourceObject` library automatically unwraps inheritance trees and is capable of providing 
ready-to-use data in `AssemblyDescriptor`-friendly format, making it cool addition to
[CelerityAssembly](../CelerityAssembly/README.md).
