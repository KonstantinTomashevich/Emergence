register_concrete (CelerityPhysics2dLogicBox2d)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dModel Log
        CONCRETE_INTERFACE CelerityPhysics2dModelBox2d CelerityTransformLogic
        THIRD_PARTY box2d)
concrete_implements_abstract (CelerityPhysics2dLogic)
