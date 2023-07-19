register_abstract (WarehouseTestsVisualization)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}/Visualization")
abstract_require (CONCRETE_INTERFACE WarehouseVisualization)

abstract_get_implementations (ABSTRACT Warehouse OUTPUT IMPLEMENTATIONS)
foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    abstract_register_implementation (NAME ${IMPLEMENTATION} PARTS WarehouseTestsVisualization${IMPLEMENTATION})
endforeach ()

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_concrete (WarehouseTestsVisualization${IMPLEMENTATION})
    concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Visualization${IMPLEMENTATION}")
    concrete_sources ("Visualization${IMPLEMENTATION}/*.cpp")
    concrete_require (SCOPE PRIVATE ABSTRACT WarehouseTestsVisualization)
endforeach ()

register_concrete (WarehouseTests)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
concrete_sources ("Private/*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT Warehouse WarehouseTestsVisualization
        CONCRETE_INTERFACE QueryTest ReferenceTest
        INTERFACE ContextExtension MemoryProfilerStub Testing)

foreach (IMPLEMENTATION ${IMPLEMENTATIONS})
    register_executable (TestWarehouse${IMPLEMENTATION})
    executable_include (
            ABSTRACT
            Assert=SDL3 CPUProfiler=None Hashing=XXHash Log=SPDLog Memory=Original MemoryProfiler=Original
            RecordCollection=Pegasus StandardLayoutMapping=Original
            Warehouse=${IMPLEMENTATION} WarehouseTestsVisualization=${IMPLEMENTATION}

            CONCRETE
            Container Handling QueryTest RecordCollectionVisualization ReferenceTest
            Threading Time VisualGraph WarehouseTests WarehouseVisualization)
    executable_verify ()
    executable_copy_linked_artefacts ()

    add_test (NAME "TestWarehouse${IMPLEMENTATION}" COMMAND TestWarehouse${IMPLEMENTATION})
    add_dependencies (EmergenceTests TestWarehouse${IMPLEMENTATION})
endforeach ()
