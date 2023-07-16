set (GAME_NAME "Platformer2dDemo")
option (EMERGENCE_PLATFORMED_2D_GAME_SHIPPING "Whether Platformer2dDemo game should be built in shipping variant." OFF)
set (GAME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Games/${GAME_NAME}")

file (MAKE_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")
file (MAKE_DIRECTORY "${GAME_OUTPUT_DIRECTORY}Packaged")
add_subdirectory (Unit)

# TODO: Try to move common content into a shared library.

register_executable (${GAME_NAME}ResourceCooker)
executable_include (
        ABSTRACT
        Assert=SDL3 CelerityPhysics2d=Box2d CelerityUI=ImGUI CPUProfiler=None Hashing=XXHash JobDispatcher=Original
        Log=SPDLog Memory=Original MemoryProfiler=Original RecordCollection=Pegasus RenderBackend=BGFX
        ResourceProvider=Original StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original
        Warehouse=Galleon

        CONCRETE
        ${GAME_NAME}Logic ${GAME_NAME}Model ${GAME_NAME}ResourceCookerApplication ${GAME_NAME}Resources Celerity
        CelerityAsset CelerityAssembly CelerityInput CelerityLocale CelerityPhysics2dDebugDraw CelerityRenderFoundation
        CelerityResourceConfig CelerityRender2d CelerityResourceObject CelerityTransform CommandLine Container Flow
        Handling Math RecordCollectionVisualization ResourceCooking ResourceObject Serialization SyntaxSugar
        TaskCollection VisualGraph)

executable_verify ()
executable_copy_linked_artefacts ()
set_target_properties ("${GAME_NAME}ResourceCooker" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")

register_executable (${GAME_NAME}Game)
executable_include (
        ABSTRACT
        Assert=SDL3 CelerityPhysics2d=Box2d CelerityUI=ImGUI Hashing=XXHash JobDispatcher=Original Log=SPDLog
        Memory=Original RecordCollection=Pegasus RenderBackend=BGFX ResourceProvider=Original
        StandardLayoutMapping=Original TaskExecutor=Parallel VirtualFileSystem=Original Warehouse=Galleon

        CONCRETE
        ${GAME_NAME}Logic ${GAME_NAME}Model ${GAME_NAME}GameApplication ${GAME_NAME}Resources Celerity
        CelerityAsset CelerityAssembly CelerityInput CelerityLocale CelerityPhysics2dDebugDraw CelerityRenderFoundation
        CelerityResourceConfig CelerityRender2d CelerityResourceObject CelerityTransform Container Flow Handling Math
        MemoryRecording RecordCollectionVisualization ResourceObject Serialization SyntaxSugar TaskCollection
        VisualGraph)

if (EMERGENCE_PLATFORMED_2D_GAME_SHIPPING)
    executable_include (ABSTRACT CPUProfiler=None MemoryProfiler=None)
else ()
    executable_include (ABSTRACT CPUProfiler=Tracy MemoryProfiler=Original)
endif ()

executable_verify ()
executable_copy_linked_artefacts ()

executable_deploy_resource_mount_lists ("${GAME_OUTPUT_DIRECTORY}")
executable_setup_resource_cooking_and_packaging (
        COOKER ${GAME_NAME}ResourceCooker
        COOKING_WORKSPACE "${GAME_OUTPUT_DIRECTORY}/CookingWorkspace"
        MOUNT_LIST_DIRECTORY ${GAME_OUTPUT_DIRECTORY}
        PACKAGING_OUTPUT ${GAME_OUTPUT_DIRECTORY}Packaged)
set_target_properties ("${GAME_NAME}Game" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIRECTORY}")
