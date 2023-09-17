# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_concrete (CelerityPhysics3dLogicPhysX)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics3dModel Log
        CONCRETE_INTERFACE CelerityPhysics3dModelPhysX CelerityTransformLogic
        THIRD_PARTY PhysX)
concrete_implements_abstract (CelerityPhysics3dLogic)
