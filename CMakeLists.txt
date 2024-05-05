cmake_minimum_required(VERSION 3.20)

set(ROOT_PROJECT_NAME "FLHook")
set(PROJECT_NAME ${ROOT_PROJECT_NAME})

project("${PROJECT_NAME}" LANGUAGES CXX VERSION 5.0.0)

message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Only the Clang compiler is currently supported for FLHook")
endif()

# Define our dependencies function for after
include("Dependencies.cmake")

# Ensure reflect-cpp doesn't fail with complex structs
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /clang:-ftemplate-depth=2048 /clang:-fconstexpr-depth=2048 /clang:-fconstexpr-steps=80000 /EHa")

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR})
set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source" )
set(FLHOOK_COPY_PATH "$ENV{FLHOOK_COPY_PATH}" )
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
set(SDK_PATH "${PROJECT_SOURCE_DIR}/vendor/FLHookSDK")
set(refl "${PROJECT_SOURCE_DIR}/vendor/reflect-cpp")

if (${CMAKE_GENERATOR} STREQUAL "Ninja")
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
    set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
else()
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE_INIT})
    set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE_INIT}/plugins)
endif()

file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false
        "${SOURCE_PATH}/*.c*"
        "${SOURCE_PATH}/*.h*"
        "${INCLUDE_PATH}/*.c*"
        "${INCLUDE_PATH}/*.h*"
        "${SDK_PATH}/*.h*"
        "${refl}/*.hpp"
)

add_library("${PROJECT_NAME}" SHARED ${SOURCE_FILES} include/PCH.hpp include/PCH.cpp)
set_target_properties("${PROJECT_NAME}" PROPERTIES
        LINKER_LANGUAGE CXX
        ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH}/External/uvw)
target_dependencies(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;"
        "_CRT_SECURE_NO_WARNINGS;"
        "WIN32_LEAN_AND_MEAN;"
        "UNICODE;_UNICODE;"
        "USE_GLM;"
        "NOMINMAX;"
        "SERVER;"
        "FLHOOK;"
)

message(STATUS "Set binary output to: ${OUTPUT_DIR}")

find_package(Python3 COMPONENTS Interpreter)
if (${Python3_FOUND} STREQUAL "TRUE" AND ${Python3_Interpreter_FOUND} STREQUAL "TRUE")
    message(STATUS "Adding post build step")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/post_build.py -o ${OUTPUT_DIR}
    )
else()
    message(STATUS "Python3 interpreter not found. Not adding post build step.")
endif()

# Load plugins
message(STATUS "Adding Plugins")
file(GLOB PLUGIN_DIRECTORIES LIST_DIRECTORIES TRUE ${PROJECT_SOURCE_DIR}/plugins/*)
foreach(PLUGIN_DIR ${PLUGIN_DIRECTORIES})
    if (IS_DIRECTORY ${PLUGIN_DIR} AND EXISTS ${PLUGIN_DIR}/CMakeLists.txt)
        add_subdirectory(${PLUGIN_DIR})
    endif()
endforeach()