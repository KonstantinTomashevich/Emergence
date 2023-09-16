# Celerity<sup>Concrete</sup>

## Brief

This unit provides an ECS-like framework for game world logic development, based on [Warehouse](../Warehouse/README.md),
[Flow](../Flow/README.md) and [TaskExecutor](../TaskExecutor/README.md). `Celerity` provides a lot of optional features
through units with `Celerity` prefix in their name.

## Extension unit structure

There are two types of extensions: features, that extend simulation by adding data structures and task executors, and
frameworks, that add more top-level management on top of `World` architecture.

Every feature consists of two units: logic and model. Logic unit contains task executors and utility for them. Model
contains data structures and their utility. Main goal is this split is to make hot reload more performant: if only logic
units were changed, we shouldn't need to recreate the world views and reimport the data, while model changes result
in data reimport. Therefore, when splitting extension to logic and model parts, you should keep in mind that logic
part should be easy to reload and should not contain anything that will stop work if only pipelines were regenerated.

## Extension list: Features

We keep feature documentation here, because there is 2 units for every feature: logic and model, therefore it looks
better to keep there documentation inside Celerity.

### CelerityAssembly

Implements `Assembly` pattern: fills objects with `PrototypeComponent` with components and sub-objects from specified
assembly descriptor. This pattern provides universal and flexible way to spawn complex objects that can be altered by
non-programmers through assets. Descriptors are built on top
of [StandardLayoutMapping](../StandardLayoutMapping/README.md) patches and therefore can be both loaded from resources
and created during runtime.

### CelerityAsset

Implements asset caching behaviour: tracks asset references from long term objects using reflection and creates
asset nodes on demand. These asset nodes can be then picked up by actual asset loaders. Node usage count is updated
automatically and unused nodes will be cleaned up if special option is enabled in `AssetManagerSingleton`.

### CelerityInput

Provides input abstraction layer for game-level input processing:

- Gameplay reads input through `InputAction`s that are added to objects using `InputActionComponent`s.
  `InputAction` is a pair of group and unique id that has meaning tied to game logic and is independent
  of physical inputs.
- `InputActionComponent` may be manually added by other features (like AI)
  or received through subscription on input dispatch, see `InputSubscriptionComponent`.
- Input trigger feature is supported: input actions can be generated automatically from physical input using
  configuration from provided triggers like `KeyTrigger`.
- Physical input is expected to be passed through `FrameInputAccumulator` layer.

### CelerityLocale

Loads and manages locale-specific data like localized strings. Target locale can be freely switched in runtime: it will
just override previous one.

### CelerityPhysics2d

Adds components and tasks for 2d physical simulation to `Celerity`.

### CelerityPhysics2dDebugDraw

Adds debug draw logic for `CelerityPhysics2d` using `CelerityRender2d`.

### CelerityPhysics3d

Adds components and tasks for 3d physical simulation to `Celerity`.

### CelerityRender2d

Provides 2d rendering features on top of [RenderBackend](../RenderBackend/README.md) and `CelerityRenderFoundation`:

- Sprite rendering with batching.
- Camera with attachment support.
- UV-based animations for 2d sprites.

### CelerityRenderFoundation

Contains common features used by different render extensions:

- Materials.
- Material instances.
- Textures.
- Viewports.

Maintains common flow for loading of material, material instance and texture assets and for rendering.

### CelerityResourceConfig

This library provides support for game config resources. Config resource is an instance of particular type (for example,
`UnitConfig` or `BuildingConfig`), that is serialized to file in binary or YAML format. `CelerityResourceConfig`
provides API for loading configs by their type, which allows user to omit config storage details in game code: loading
is done through ResourceProvider, therefore paths are resolved automatically. Also, every config is loaded into the
game world as long term object and might be indexed in any way user needs it.

### CelerityResourceObject

Integrates `CelerityAssembly` with [ResourceObject](../ResourceObject/README.md) format by providing message-driven API
for loading objects and storing them as `AssemblyDescriptor`s.

### CelerityTransform

Implements transform nodes hierarchy for `Celerity`. Has different transforms for logical and visual updates and
supports interpolation-driven sync from logical to visual transform. Both 2d and 3d transform is supported.

### CelerityUI

Provides native way for setting up UI viewports with windows and simplistic controls. Uses `CelerityInput` to pass user
input from UI to game logic. `CelerityRenderFoundation` is used to render the UI geometry.

## Extension list: Frameworks

### CelerityNexus

`CelerityNexus` is an experimental framework which primary goal is to bring hot reload support to `Celerity`.
Currently, it is left in an intermediate state with logic hot reload support and without model hot reload.
