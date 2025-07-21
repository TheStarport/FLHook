set(PLUGIN_NAME Cloak)
set(PLUGIN_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PLUGIN_VERSION 1.0.0)
set(PLUGIN_IS_EXAMPLE FALSE)

include("${PLUGIN_PATH}/../Plugin.cmake")
target_compile_definitions(Cloak PRIVATE CLOAK_PLUGIN=1)