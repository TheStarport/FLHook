cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "FLHook")

project("${PROJECT_NAME}" LANGUAGES CXX)

message(STATUS "Detected compiler: ${CMAKE_CXX_COMPILER}")

set(PROJECT_VERSION 5.0.0)
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Only the Clang compiler is currently supported for FLHook")
endif()

set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source" )
set(FLHOOK_COPY_PATH "$ENV{FLHOOK_COPY_PATH}" )
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
set(SDK_PATH "${PROJECT_SOURCE_DIR}/vendor/FLHookSDK")
set(refl "${PROJECT_SOURCE_DIR}/vendor/reflect-cpp")

file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false
        "${SOURCE_PATH}/*.c*"
        "${SOURCE_PATH}/*.h*"
        "${INCLUDE_PATH}/*.c*"
        "${INCLUDE_PATH}/*.h*"
        "${SDK_PATH}/*.h*"
        "${refl}/*.hpp"
)

add_library("${PROJECT_NAME}" SHARED ${SOURCE_FILES} include/PCH.hpp include/PCH.cpp)
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;"
        "_CRT_SECURE_NO_WARNINGS;"
        "WIN32_LEAN_AND_MEAN;"
        "UNICODE;_UNICODE;"
        "NOMINMAX;"
        "SERVER;"
        "FLHOOK;"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(${PROJECT_NAME} PRIVATE ${refl}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_PATH}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_PATH}/vendor)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH}/External/uvw)

# We need the packaged YYJSON
target_include_directories(${PROJECT_NAME} PRIVATE ${refl}/src)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.hpp)

## vcpkg dependencies

find_package(re2 CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE re2::re2)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE magic_enum::magic_enum)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE spdlog::spdlog)

find_package(glm CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE glm::glm)

find_package(amqpcpp CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE amqpcpp)

find_package(uvw CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE uvw::uvw)

find_package(stduuid CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE stduuid)

find_package(xbyak CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE xbyak::xbyak)

# MongoCXX
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE mongo::bsoncxx_shared)
target_link_libraries(${PROJECT_NAME} PRIVATE mongo::mongocxx_shared)

# FLCore

target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreCommon.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreDACom.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreDALib.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreFLServerEXE.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreRemoteClient.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDK_PATH}/lib/FLCoreServer.lib")

# Build Scripts

find_package(Python3 COMPONENTS Interpreter)
if (${Python3_FOUND} STREQUAL "TRUE" AND ${Python3_Interpreter_FOUND} STREQUAL "TRUE")
    message(STATUS "Adding post build step")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/scripts/post_build.py -o ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE_INIT}
    )
else()
    message(STATUS "Python3 interpreter not found. Not adding post build step.")
endif()