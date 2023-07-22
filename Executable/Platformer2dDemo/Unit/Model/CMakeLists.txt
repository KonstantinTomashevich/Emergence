register_concrete (${GAME_NAME}Model)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dModel CelerityUIModel
        CONCRETE_INTERFACE
        CelerityAssemblyModel CelerityLocaleModel CelerityRender2dModel CelerityRenderFoundationModel
        CelerityResourceObjectModel CelerityTransformModel ResourceObject)
concrete_require (SCOPE PUBLIC CONCRETE_INTERFACE CelerityAssetModel)
