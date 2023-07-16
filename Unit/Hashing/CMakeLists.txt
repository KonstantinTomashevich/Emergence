register_abstract (Hashing)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (INTERFACE APICommon)
abstract_register_implementation (NAME XXHash PARTS HashingXXHash)
