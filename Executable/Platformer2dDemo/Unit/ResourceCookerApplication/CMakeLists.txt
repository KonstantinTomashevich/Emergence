register_concrete (${GAME_NAME}ResourceCookerApplication)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (SCOPE PRIVATE
        ABSTRACT Log
        CONCRETE_INTERFACE ${GAME_NAME}Model ${GAME_NAME}Resources CommandLine ResourceCooking Serialization
        INTERFACE ResourceProviderHelpers VirtualFileSystemHelpers)
