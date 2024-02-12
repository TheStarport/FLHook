cmake_minimum_required(VERSION 3.24)
set(PROJECT_NAME "FLHook")

project("${PROJECT_NAME}" VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER_ID "MSVC")
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source" )
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
set(SDK_PATH "${PROJECT_SOURCE_DIR}/vendor/FLHookSDK")
set(refl "${PROJECT_SOURCE_DIR}/vendor/reflect-cpp")


file(GLOB_RECURSE SOURCE_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.c*" "${SOURCE_PATH}/*.h*" "${INCLUDE_PATH}/*.c*" "${INCLUDE_PATH}/*.h*" "${INCLUDE_PATH}/*.h*" "${SDK_PATH}/*.h*" "${SDK_PATH}/*.c*" "${refl}/*.hpp" "${refl}/*.hpp"  )

add_library("${PROJECT_NAME}" ${SOURCE_FILES} include/PCH.hpp include/PCH.cpp)
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING;"
        "_CRT_SECURE_NO_WARNINGS;"
        "NOMINMAX;"
        )
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/std:c++20")
else()
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_include_directories(${PROJECT_NAME} PRIVATE ${refl}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_PATH}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDK_PATH}/vendor)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH}/External/uvw)

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

find_package(libuv CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>)

