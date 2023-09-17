# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_concrete (CelerityPhysics3dModelPhysX)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT JobDispatcher
        THIRD_PARTY PhysX)
concrete_implements_abstract (CelerityPhysics3dModel)
