cmake_minimum_required(VERSION 3.20)

set(ROOT_PROJECT_NAME "FLHook")
set(PROJECT_NAME ${ROOT_PROJECT_NAME})

project("${PROJECT_NAME}" LANGUAGES CXX VERSION 5.0.0)

message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 20)

# Define our dependencies function for after
include("Dependencies.cmake")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /cgthreads8")

set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source" )
set(FLHOOK_COPY_PATH "$ENV{FLHOOK_COPY_PATH}" )
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
set(SDK_PATH "${PROJECT_SOURCE_DIR}/vendor/FLHookSDK")
set(VENDOR "${PROJECT_SOURCE_DIR}/vendor")

set(PROJECT_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(PLUGIN_OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/plugins)

file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false
        "${SOURCE_PATH}/*.c*"
        "${SOURCE_PATH}/*.h*"
        "${INCLUDE_PATH}/*.c*"
        "${INCLUDE_PATH}/*.h*"
        "${SDK_PATH}/*.h*"
        "${VENDOR}/reflect-cpp/*.hpp"
)

add_library("${PROJECT_NAME}" SHARED ${SOURCE_FILES} include/PCH.hpp include/PCH.cpp)
set_target_properties("${PROJECT_NAME}" PROPERTIES
        LINKER_LANGUAGE CXX
        ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_OUTPUT_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_OUTPUT_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT_DIR}
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
        "REFLECT_CPP_NO_C_ARRAYS_OR_INHERITANCE;"
)

macro(SET_POST_BUILD CUR_TARGET)
    find_package(Python3 COMPONENTS Interpreter)
    if (${Python3_FOUND} STREQUAL "TRUE" AND ${Python3_Interpreter_FOUND} STREQUAL "TRUE")
        message(STATUS "Adding post build step")
        add_custom_command(TARGET ${CUR_TARGET} POST_BUILD
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/post_build.py -o ${PROJECT_OUTPUT_DIR}
        )
    else()
        message(STATUS "Python3 interpreter not found. Not adding post build step.")
    endif()
endmacro()

set_post_build(${PROJECT_NAME})

# Load plugins
file(GLOB PLUGIN_DIRECTORIES LIST_DIRECTORIES TRUE ${PROJECT_SOURCE_DIR}/plugins/*)
foreach(PLUGIN_DIR ${PLUGIN_DIRECTORIES})
    if (NOT IS_DIRECTORY ${PLUGIN_DIR})
        continue()
    endif()

    if (${PLUGIN_DIR} MATCHES ".*/_examples" AND COMPILE_EXAMPLES)
        file(GLOB DIRS LIST_DIRECTORIES TRUE ${PLUGIN_DIR}/*)
        foreach(M_DIR ${DIRS})
            if (NOT IS_DIRECTORY ${M_DIR})
                continue()
            endif()

            if(EXISTS ${M_DIR}/CMakeLists.txt)
                add_subdirectory(${M_DIR})
            endif()
        endforeach()
    elseif(EXISTS ${PLUGIN_DIR}/CMakeLists.txt AND COMPILE_PLUGINS)
        add_subdirectory(${PLUGIN_DIR})
    endif()
endforeach()