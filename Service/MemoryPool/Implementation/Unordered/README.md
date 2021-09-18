# MemoryPool::Unordered

[MemoryPool](../../README.md) implementation, that uses unordered chunk selection strategy. Therefore, allocation and
deallocation operations are fast, but iteration is usually a lot slower than with ordered chunk selection, also new
object allocation address is generally random and therefore not as cache coherent as it could be.
