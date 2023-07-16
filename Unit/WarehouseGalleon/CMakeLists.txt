register_concrete (WarehouseGalleon)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PRIVATE ABSTRACT Warehouse CONCRETE_INTERFACE Galleon RecordCollectionVisualization)
