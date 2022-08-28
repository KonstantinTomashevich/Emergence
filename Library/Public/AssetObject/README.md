### AssetObject

This library provides format for storing game objects (like units, buildings, etc) in YAML or binary. It is designed
to be used with [Celerity::Assembly](../Celerity/Extension/Assembly/README.md), but can be used as separate library too.

Objects consist of parts, like entities in ECS, and each part is defined by
[StandardLayoutMapping](../../../Service/StandardLayoutMapping/README.md) patch, much like
[Celerity::Assembly](../Celerity/Extension/Assembly/README.md) defines components in `AssemblyDescriptor`.
In contrast to `AssemblyDescriptor`s, objects support single inheritance: for example, you can create base object
that suits any unit, then extend it with components in child objects that are created for every unit. And then, you
can create child classes with unit skins too! `AssetObject` library automatically unwraps inheritance trees and
is capable of providing ready-to-use data in `AssemblyDescriptor`-friendly format, making it cool addition to
[Celerity::Assembly](../Celerity/Extension/Assembly/README.md).

In addition, `AssetObject` library uses folder system to load packages of objects instead of on-demand loading. It 
allows to load all required objects at once during level loading, but also expects user to know which packages are
needed for the selected level. Every folder may declare list of dependencies: folders that should be loaded along
with this folder, for example folder with common base objects. It makes package selection more flexible as user only
needs to know which top-level packages game needs, because lower level packages will be loaded automatically as
dependencies. Also, folder system is used to automatically generate object names from object paths: object path
is essentially `{PathToFolder}/{ObjectName}{Suffix}` where object name is allowed to contain `/` symbols too, making
use of subdirectories as namespaces possible.