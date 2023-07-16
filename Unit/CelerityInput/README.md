# CelerityInput<sup>Concrete</sup>

Provides input abstraction layer for game-level input processing:

- Gameplay reads input through `InputAction`s that are added to objects using `InputActionComponent`s.
  `InputAction` is a pair of group and unique id that has meaning tied to game logic and is independent
  of physical inputs.
- `InputActionComponent` may be manually added by other features (like AI)
  or received through subscription on input dispatch, see `InputSubscriptionComponent`.
- Input trigger feature is supported: input actions can be generated automatically from physical input using
  configuration from provided triggers like `KeyTrigger`.
- Physical input is expected to be passed through `FrameInputAccumulator` layer.
