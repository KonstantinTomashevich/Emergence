register_concrete ("${GAME_NAME}Logic")
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dLogic CelerityUILogic Log

        CONCRETE_INTERFACE
        ${GAME_NAME}Model CelerityLocaleLogic CelerityPhysics2dDebugDrawLogic CelerityRender2dLogic
        CelerityResourceObjectLogic CelerityTransformLogic CelerityResourceObjectLogic Serialization Time)

concrete_require (
        SCOPE PUBLIC
        CONCRETE_INTERFACE
        Celerity CelerityAssetLogic CelerityAssemblyLogic CelerityInputLogic CelerityResourceConfigLogic ResourceObject)
