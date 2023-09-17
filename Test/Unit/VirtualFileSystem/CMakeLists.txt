register_concrete (VirtualFileSystemTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT VirtualFileSystem INTERFACE MemoryProfilerStub Testing)

abstract_get_implementations (ABSTRACT VirtualFileSystem OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestVirtualFileSystem${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original MemoryProfiler=Original
            RecordCollection=Pegasus StandardLayoutMapping=Original VirtualFileSystem=${IMPLEMENTATION}

            CONCRETE Container Handling VirtualFileSystemTests Serialization Threading Time)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestVirtualFileSystem${IMPLEMENTATION}" COMMAND TestVirtualFileSystem${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestVirtualFileSystem${IMPLEMENTATION})
endforeach ()
