register_abstract (Warehouse)
abstract_include ("${CMAKE_CURRENT_SOURCE_DIR}")
abstract_require (ABSTRACT Memory StandardLayoutMapping CONCRETE_INTERFACE Container VisualGraph INTERFACE APICommon)
abstract_register_implementation (NAME Galleon PARTS Galleon WarehouseGalleon)
