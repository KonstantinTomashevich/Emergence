register_abstract (Box2dAdapter)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_register_implementation (NAME Efficient PARTS Box2dAdapterEfficient)
abstract_register_implementation (NAME Profiled PARTS Box2dAdapterProfiled)
