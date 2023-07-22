register_abstract (CelerityPhysics2dModel)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE Celerity)
abstract_register_implementation (NAME Box2d PARTS CelerityPhysics2dModelBox2d CelerityPhysics2dModelCommon)
