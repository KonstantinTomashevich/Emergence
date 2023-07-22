register_abstract (CelerityPhysics2dLogic)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT CelerityPhysics2dModel CONCRETE_INTERFACE Celerity)
abstract_register_implementation (NAME Box2d PARTS CelerityPhysics2dLogicBox2d)
