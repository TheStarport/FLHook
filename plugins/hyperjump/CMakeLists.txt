set(PLUGIN_NAME Hyperjump)
set(PLUGIN_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PLUGIN_VERSION 1.0.0)
set(PLUGIN_IS_EXAMPLE FALSE)

include("${PLUGIN_PATH}/../Plugin.cmake")
target_compile_definitions(Hyperjump PRIVATE HYPERJUMP_PLUGIN=1)