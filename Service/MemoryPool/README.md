# MemoryPool

MemoryPool service provides paged memory allocator interface, that helps to decrease memory fragmentation and increase
cache coherency for objects of selected type. Some implementations also provide faster (in average) allocation and
deallocation operations than default malloc and free.
