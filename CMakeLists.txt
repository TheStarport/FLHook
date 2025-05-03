cmake_minimum_required(VERSION 3.20)

project("${PROJECT_NAME}" LANGUAGES CXX VERSION 5.0.0)
message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions("WINAPI_FAMILY=100")
    if (NOT DEFINED ENV{MSVC})
        message(FATAL_ERROR "MSVC env was not found and running on Linux")
    endif ()

    if (NOT DEFINED ENV{INCLUDE})
        message(FATAL_ERROR "'INCLUDE' env not found. Have you run 'source $ENV{MSVC}/bin/x86/msvcenv.sh' ?")
    endif ()

    string(REPLACE "z:\\" "/" MSVC_INCLUDE "$ENV{INCLUDE}")
    string(REPLACE "\\" "/" MSVC_INCLUDE "${MSVC_INCLUDE}")
    string(REPLACE ";" " " MSVC_INCLUDE "${MSVC_INCLUDE}")
    separate_arguments(MSVC_INCLUDE)
    include_directories(${MSVC_INCLUDE})
endif ()

# Starting variables / paths
set(ROOT_PROJECT_NAME "FLHook")
set(PROJECT_NAME ${ROOT_PROJECT_NAME})
set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source")
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
set(SDK_PATH "${PROJECT_SOURCE_DIR}/vendor/FLHookSDK")
set(VENDOR "${PROJECT_SOURCE_DIR}/vendor")

set(PROJECT_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(PLUGIN_OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/plugins)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /cgthreads8")

# Load cache variables from
include("LoadConf.cmake")

# Define our dependencies function for after
include("Dependencies.cmake")

file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false
        "${SOURCE_PATH}/*.c*"
        "${SOURCE_PATH}/*.h*"
        "${INCLUDE_PATH}/*.c*"
        "${INCLUDE_PATH}/*.h*"
        "${SDK_PATH}/*.h*"
        "${VENDOR}/reflect-cpp/*.hpp"
)

find_package(msgpack)
find_package(mongoc-1.0 CONFIG REQUIRED)

set(REFLECTCPP_BUILD_SHARED OFF)
set(REFLECTCPP_MSGPACK ON)
set(REFLECTCPP_BSON ON)
add_subdirectory(${VENDOR}/reflect-cpp)

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
    if (NOT DEFINED NO_POST_BUILD AND DEFINED BINARY_COPY_DESTINATION)
        find_package(Python3 COMPONENTS Interpreter)
        if (${Python3_FOUND} STREQUAL "TRUE" AND ${Python3_Interpreter_FOUND} STREQUAL "TRUE")
            message(STATUS "Adding post build step")
            if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
                set(RELEASE_FLAG "--release")
            endif ()
            add_custom_command(TARGET ${CUR_TARGET} POST_BUILD
                    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/post_build.py
                    -d "${BINARY_COPY_DESTINATION}" ${RELEASE_FLAG}
            )
        else ()
            message(STATUS "Python3 interpreter not found. Not adding post build step.")
        endif ()
    endif ()
endmacro()

set_post_build(${PROJECT_NAME})

# Load plugins
file(GLOB PLUGIN_DIRECTORIES LIST_DIRECTORIES TRUE ${PROJECT_SOURCE_DIR}/plugins/*)
foreach (PLUGIN_DIR ${PLUGIN_DIRECTORIES})
    if (NOT IS_DIRECTORY ${PLUGIN_DIR})
        continue()
    endif ()

    if (${PLUGIN_DIR} MATCHES ".*/_examples" AND COMPILE_EXAMPLES)
        file(GLOB DIRS LIST_DIRECTORIES TRUE ${PLUGIN_DIR}/*)
        foreach (M_DIR ${DIRS})
            if (NOT IS_DIRECTORY ${M_DIR})
                continue()
            endif ()

            if (EXISTS ${M_DIR}/CMakeLists.txt)
                add_subdirectory(${M_DIR})
            endif ()
        endforeach ()
    elseif (EXISTS ${PLUGIN_DIR}/CMakeLists.txt AND COMPILE_PLUGINS)
        add_subdirectory(${PLUGIN_DIR})
    endif ()
endforeach ()