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
        Assert=SDL3 Hashing=XXHash JobDispatcher=Original Log=SPDLog Memory=Original RecordCollection=Pegasus
        RenderBackend=BGFX ResourceProvider=Original StandardLayoutMapping=Original TaskExecutor=Parallel
        VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        Celerity Container Flow Handling Math MemoryRecording RecordCollectionVisualization ResourceObject
        Serialization TaskCollection Threading Time VisualGraph)

if (EMERGENCE_PLATFORMED_2D_GAME_SHIPPING)
    shared_library_include (SCOPE PUBLIC ABSTRACT CPUProfiler=None MemoryProfiler=None)
else ()
    shared_library_include (SCOPE PUBLIC ABSTRACT CPUProfiler=Tracy MemoryProfiler=Original)
endif ()

shared_library_verify ()

register_shared_library (${GAME_NAME}SharedGame)
shared_library_include (
        SCOPE PUBLIC

        ABSTRACT
        CelerityPhysics2d=Box2d CelerityUI=ImGUI

        CONCRETE
        ${GAME_NAME}Logic ${GAME_NAME}Model ${GAME_NAME}Resources CelerityAsset CelerityAssembly CelerityInput
        CelerityLocale CelerityPhysics2dDebugDraw CelerityRenderFoundation CelerityResourceConfig CelerityRender2d
        CelerityResourceObject CelerityTransform)
shared_library_link_shared_library (PUBLIC "${GAME_NAME}SharedBase")
shared_library_verify ()

register_executable (${GAME_NAME}ResourceCooker)
executable_include (CONCRETE ${GAME_NAME}ResourceCookerApplication CommandLine ResourceCooking)
executable_link_shared_libraries (${GAME_NAME}SharedGame)
executable_verify ()
executable_copy_linked_artefacts ()
set_target_properties ("${GAME_NAME}ResourceCooker" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")

register_executable (${GAME_NAME}Game)
executable_include (CONCRETE ${GAME_NAME}GameApplication)
executable_link_shared_libraries (${GAME_NAME}SharedGame)
executable_verify ()
executable_copy_linked_artefacts ()

executable_deploy_resource_mount_lists ("${GAME_OUTPUT_DIRECTORY}")
executable_setup_resource_cooking_and_packaging (
        COOKER ${GAME_NAME}ResourceCooker
        COOKING_WORKSPACE "${GAME_OUTPUT_DIRECTORY}/CookingWorkspace"
        MOUNT_LIST_DIRECTORY ${GAME_OUTPUT_DIRECTORY}
        PACKAGING_OUTPUT ${GAME_OUTPUT_DIRECTORY}Packaged)
set_target_properties ("${GAME_NAME}Game" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")
