function(TARGET_DEPENDENCIES PROJ)
    if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
        if (NOT DEFINED ENV{MSVC_INCLUDE})
            message(FATAL_ERROR "MSVC_INCLUDE was not found and running on Linux")
        endif ()
        string(REPLACE ":" " " MSVC_INCLUDES $ENV{MSVC_INCLUDE})
        separate_arguments(MSVC_INCLUDES)
        target_include_directories(${PROJ} PRIVATE ${MSVC_INCLUDES})
    endif ()

    target_include_directories(${PROJ} PRIVATE ${INCLUDE_PATH})
    target_include_directories(${PROJ} PRIVATE ${refl}/include)
    target_include_directories(${PROJ} PRIVATE ${SDK_PATH}/include)
    target_include_directories(${PROJ} PRIVATE ${SDK_PATH}/vendor)

    # Add Wildcards submodule
    target_include_directories(${PROJ} PRIVATE ${VENDOR}/wildcards/include)

    # Add CPP Result submodule
    target_include_directories(${PROJ} PRIVATE ${VENDOR}/result/include)

    # Add ReflectCPP
    target_include_directories(${PROJ} PRIVATE ${VENDOR}/reflect-cpp/src)
    target_include_directories(${PROJ} PRIVATE ${VENDOR}/reflect-cpp/include)
    target_include_directories(${PROJ} PRIVATE ${VENDOR}/reflect-cpp/include/rfl/thirdparty)
    target_link_libraries(${PROJ} PRIVATE reflectcpp)

    # PCH
    target_precompile_headers(${PROJ} PRIVATE ${INCLUDE_PATH}/PCH.hpp)

    # Concurrencpp
    target_link_libraries(${PROJ} PRIVATE concurrencpp::concurrencpp)

    # conan dependencies
    find_package(concurrentqueue CONFIG REQUIRED)
    find_package(cpptrace CONFIG REQUIRED)
    find_package(croncpp CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_PACKAGE(httplib CONFIG REQUIRED)
    find_package(magic_enum CONFIG REQUIRED)
    find_package(OpenSSL CONFIG REQUIRED)
    find_package(stduuid CONFIG REQUIRED)
    find_package(xbyak CONFIG REQUIRED)
    find_package(zstd REQUIRED)
    target_link_libraries(${PROJ} PUBLIC concurrentqueue::concurrentqueue cpptrace::cpptrace croncpp::croncpp glm::glm httplib::httplib
            magic_enum::magic_enum openssl::openssl stduuid::stduuid xbyak::xbyak zstd::libzstd_static)

    # Lua
    find_package(lua CONFIG REQUIRED)
    find_package(sol2 CONFIG REQUIRED)
    target_link_libraries(${PROJ} PUBLIC sol2::sol2 lua::lua)
    target_compile_definitions(${PROJ} PUBLIC SOL_ALL_SAFETIES_ON=1)

    # MongoCXX
    find_package(mongocxx CONFIG REQUIRED)
    find_package(mongoc-1.0 CONFIG REQUIRED)
    target_link_libraries(${PROJ} PUBLIC mongo::mongoc_shared mongo::mongocxx_shared)

    # FLCore
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreCommon.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreDACom.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreDALib.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreFLServerEXE.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreRemoteClient.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreServer.lib")
endfunction()