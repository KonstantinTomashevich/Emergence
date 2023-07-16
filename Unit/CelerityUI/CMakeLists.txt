register_abstract (CelerityUI)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE CelerityRenderFoundation CelerityInput)
abstract_register_implementation (NAME ImGUI PARTS CelerityUICommon CelerityUIImGUI)
