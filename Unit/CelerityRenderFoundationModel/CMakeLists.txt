register_concrete (CelerityRenderFoundationModel)
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")
concrete_require (SCOPE PUBLIC ABSTRACT RenderBackend CONCRETE_INTERFACE Celerity CelerityAssetModel)
