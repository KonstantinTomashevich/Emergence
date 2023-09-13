set (GAME_NAME "Platformer2dDemo")
option (EMERGENCE_PLATFORMED_2D_GAME_SHIPPING "Whether Platformer2dDemo game should be built in shipping variant." OFF)
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")

file (MAKE_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")
file (MAKE_DIRECTORY "${GAME_OUTPUT_DIRECTORY}Packaged")
add_subdirectory (Unit)

register_shared_library ("${GAME_NAME}SharedBase")
shared_library_include (
        SCOPE PUBLIC

        ABSTRACT
        Assert=SDL3 Hashing=XXHash JobDispatcher=Original Log=SPDLog
        Memory=Original RecordCollection=Pegasus RenderBackend=BGFX ResourceProvider=Original
        StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        ${GAME_NAME}Base Celerity CelerityNexus Container Flow Handling InputStorage Math MemoryRecording
        RecordCollectionVisualization ResourceObject Serialization TaskCollection Threading Time VisualGraph)

if (EMERGENCE_PLATFORMED_2D_GAME_SHIPPING)
    shared_library_include (
            SCOPE PUBLIC ABSTRACT
            Box2dAdapter=Efficient CPUProfiler=None ImGUIAdapter=Efficient MemoryProfiler=None)
else ()
    shared_library_include (
            SCOPE PUBLIC ABSTRACT
            Box2dAdapter=Profiled CPUProfiler=Tracy ImGUIAdapter=Profiled MemoryProfiler=Original)
endif ()

shared_library_verify ()

register_shared_library (${GAME_NAME}SharedModel)
shared_library_include (
        SCOPE PUBLIC

        ABSTRACT
        CelerityPhysics2dModel=Box2d CelerityUIModel=ImGUI

        CONCRETE
        ${GAME_NAME}Model CelerityAssetModel CelerityAssemblyModel CelerityInputModel CelerityLocaleModel
        CelerityPhysics2dDebugDrawModel CelerityRenderFoundationModel CelerityResourceConfigModel CelerityRender2dModel
        CelerityResourceObjectModel CelerityTransformModel)
shared_library_link_shared_library (PUBLIC "${GAME_NAME}SharedBase")
shared_library_verify ()

register_shared_library (${GAME_NAME}SharedLogic)
shared_library_include (
        SCOPE PUBLIC

        ABSTRACT
        CelerityPhysics2dLogic=Box2d CelerityUILogic=ImGUI

        CONCRETE
        ${GAME_NAME}Logic ${GAME_NAME}Resources CelerityAssetLogic CelerityAssemblyLogic CelerityInputLogic
        CelerityLocaleLogic CelerityPhysics2dDebugDrawLogic CelerityRenderFoundationLogic CelerityResourceConfigLogic
        CelerityRender2dLogic CelerityResourceObjectLogic CelerityTransformLogic)
shared_library_link_shared_library (PUBLIC "${GAME_NAME}SharedModel")
shared_library_verify ()

register_executable (${GAME_NAME}ResourceCooker)
executable_include (CONCRETE ${GAME_NAME}ResourceCookerApplication ${GAME_NAME}Resources CommandLine ResourceCooking)
executable_link_shared_libraries (${GAME_NAME}SharedModel)
executable_verify ()
executable_copy_linked_artefacts ()
set_target_properties ("${GAME_NAME}ResourceCooker" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")

register_executable (${GAME_NAME}Game)
executable_include (CONCRETE ${GAME_NAME}GameApplication ${GAME_NAME}Resources)
executable_link_shared_libraries (${GAME_NAME}SharedBase)
executable_integrate_celerity_nexus (LOGIC "${GAME_NAME}SharedLogic" MODEL "${GAME_NAME}SharedModel")
executable_verify ()
executable_copy_linked_artefacts ()

group_deploy_resource_mount_lists (
        TARGETS "${GAME_NAME}Game" "${GAME_NAME}SharedLogic" "${GAME_NAME}SharedModel"
        OUTPUT "${GAME_OUTPUT_DIRECTORY}")

group_setup_resource_cooking_and_packaging (
        NAME "${GAME_NAME}"
        TARGETS "${GAME_NAME}Game" "${GAME_NAME}SharedLogic" "${GAME_NAME}SharedModel"
        COOKER ${GAME_NAME}ResourceCooker
        COOKING_WORKSPACE "${GAME_OUTPUT_DIRECTORY}/CookingWorkspace"
        MOUNT_LIST_DIRECTORY ${GAME_OUTPUT_DIRECTORY}
        PACKAGING_OUTPUT ${GAME_OUTPUT_DIRECTORY}Packaged)

celerity_nexus_add_bootstrap_to_package (
        NAME "${GAME_NAME}"
        EXECUTABLE "${GAME_NAME}Game"
        PACKAGING_OUTPUT "${GAME_OUTPUT_DIRECTORY}Packaged")

set_target_properties ("${GAME_NAME}Game" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")
