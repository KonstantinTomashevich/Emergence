# Celerity: BGFX2d extension

Implements [Render2d extension](../../../../../Service/Celerity/Extension/Render2d/README.md) using BGFX.

- Batches drawables into dynamic geometries, created through transient buffers, reducing draw call count by that.
- Automatically attaches and manages drawable global bounds system for view quad culling.
