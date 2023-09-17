register_concrete (ResourceCookingTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE CONCRETE_INTERFACE ResourceCooking Serialization INTERFACE MemoryProfilerStub Testing)

register_executable (TestResourceCooking)
executable_include (
        ABSTRACT
        Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original MemoryProfiler=Original
        RecordCollection=Pegasus ResourceProvider=Original StandardLayoutMapping=Original VirtualFileSystem=Original

        CONCRETE Container Handling ResourceCooking ResourceCookingTests Serialization Threading Time)
executable_verify ()
executable_copy_linked_artefacts ()

add_test (NAME "TestResourceCooking" COMMAND TestResourceCooking)
add_dependencies (EmergenceTests TestResourceCooking)
