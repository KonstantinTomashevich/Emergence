register_concrete (CelerityRenderFoundationLogic)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PUBLIC
        ABSTRACT ResourceProvider
        CONCRETE_INTERFACE CelerityAssetLogic CelerityRenderFoundationModel)
concrete_require (SCOPE PRIVATE ABSTRACT JobDispatcher Log)
