register_abstract (CelerityUILogic)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT CelerityUIModel CONCRETE_INTERFACE CelerityAssetModel InputStorage)
abstract_register_implementation (NAME ImGUI PARTS CelerityUILogicImGUI)
