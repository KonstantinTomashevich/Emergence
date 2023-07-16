# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_concrete (CelerityPhysics3dPhysX)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics3d JobDispatcher Log
        CONCRETE_INTERFACE CelerityTransform
        THIRD_PARTY PhysX)
