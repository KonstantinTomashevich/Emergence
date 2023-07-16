# MemoryProfilerStub

This unit provides `Memory::DefaultAllocationGroup` instance for all types, that adds these types into `TestStub`
allocation group. We don't need to profile memory in tests, therefore this approach is used to ignore it in tests.
