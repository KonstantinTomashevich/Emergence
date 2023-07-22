register_concrete (CelerityUILogicImGUI)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
concrete_sources ("*.cpp")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityUIModel JobDispatcher Log
        CONCRETE_INTERFACE
        CelerityAssetLogic CelerityLocaleLogic CelerityRenderFoundationLogic CelerityInputLogic CelerityUIModelImGUI
        THIRD_PARTY ImGUI)
concrete_implements_abstract (CelerityUILogic)
