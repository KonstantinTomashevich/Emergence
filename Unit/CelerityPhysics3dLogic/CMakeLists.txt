# PhysX is the only implementation now, so we cannot declare the abstract unit if it is not found.
if (NOT TARGET PhysX)
    return ()
endif ()

register_abstract (CelerityPhysics3dLogic)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT CelerityPhysics3dModel CONCRETE_INTERFACE Celerity)
abstract_register_implementation (NAME PhysX PARTS CelerityPhysics3dLogicPhysX)
