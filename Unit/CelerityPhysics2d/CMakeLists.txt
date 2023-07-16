register_abstract (CelerityPhysics2d)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE Celerity)
abstract_register_implementation (NAME Box2d PARTS CelerityPhysics2dBox2d CelerityPhysics2dCommon)
