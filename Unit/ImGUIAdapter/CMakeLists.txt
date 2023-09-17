register_abstract (ImGUIAdapter)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_register_implementation (NAME Efficient PARTS ImGUIAdapterEfficient)
abstract_register_implementation (NAME Profiled PARTS ImGUIAdapterProfiled)
