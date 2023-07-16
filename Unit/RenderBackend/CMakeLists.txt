register_abstract (RenderBackend)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (CONCRETE_INTERFACE Container Math INTERFACE APICommon)
abstract_register_implementation (NAME BGFX PARTS RenderBackendBGFX)
