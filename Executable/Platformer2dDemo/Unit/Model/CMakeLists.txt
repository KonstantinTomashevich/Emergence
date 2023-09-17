register_concrete (${GAME_NAME}Model)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2dModel CelerityUIModel
        CONCRETE_INTERFACE
        ${GAME_NAME}Base CelerityAssemblyModel CelerityLocaleModel CelerityNexus CelerityRender2dModel
        CelerityRenderFoundationModel CelerityResourceObjectModel CelerityTransformModel ResourceObject
        INTERFACE ResourceProviderHelpers)
concrete_require (SCOPE PUBLIC CONCRETE_INTERFACE CelerityAssetModel)
