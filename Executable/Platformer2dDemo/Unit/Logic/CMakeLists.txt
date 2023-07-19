register_concrete ("${GAME_NAME}Logic")
concrete_include (PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityPhysics2d CelerityUI Log

        CONCRETE_INTERFACE
        ${GAME_NAME}Model CelerityLocale CelerityPhysics2dDebugDraw CelerityRender2d
        CelerityResourceObject CelerityTransform CelerityResourceObject Serialization Time)

concrete_require (
        SCOPE PUBLIC
        CONCRETE_INTERFACE Celerity CelerityAsset CelerityAssembly CelerityInput CelerityResourceConfig ResourceObject)
