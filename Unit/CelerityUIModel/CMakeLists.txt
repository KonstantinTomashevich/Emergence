register_abstract (CelerityUIModel)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE CelerityRenderFoundationModel CelerityInputModel)
abstract_register_implementation (NAME ImGUI PARTS CelerityUIModelCommon CelerityUIModelImGUI)
