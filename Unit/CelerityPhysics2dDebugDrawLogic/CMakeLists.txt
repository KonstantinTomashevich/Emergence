register_concrete (CelerityPhysics2dDebugDrawLogic)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC CONCRETE_INTERFACE Celerity CelerityPhysics2dDebugDrawModel)

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dLogic CelerityPhysics2dModel Log
        CONCRETE_INTERFACE CelerityRender2dModel)

