# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_abstract (CelerityPhysics3d)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE Celerity)
abstract_register_implementation (NAME PhysX PARTS CelerityPhysics3dCommon CelerityPhysics3dPhysX)
