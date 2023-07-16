register_concrete (CelerityUIImGUI)
concrete_include (PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Private")
concrete_sources ("Private/*.cpp" "Resources/Shaders/*.sc")

concrete_require (
        SCOPE PRIVATE
        ABSTRACT CelerityUI JobDispatcher Log
        CONCRETE_INTERFACE CelerityLocale
        THIRD_PARTY ImGUI)
register_default_resource_usage_scheme ()
