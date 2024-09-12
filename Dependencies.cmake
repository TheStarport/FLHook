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

    target_precompile_headers(${PROJ} PRIVATE ${INCLUDE_PATH}/PCH.hpp)

    ## vcpkg dependencies

    find_package(amqpcpp CONFIG REQUIRED)
    find_package(concurrentqueue CONFIG REQUIRED)
    find_package(croncpp CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(magic_enum CONFIG REQUIRED)
    find_package(re2 CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
    find_package(stduuid CONFIG REQUIRED)
    find_package(uvw CONFIG REQUIRED)
    find_package(xbyak CONFIG REQUIRED)
    target_link_libraries(${PROJ} PUBLIC amqpcpp concurrentqueue::concurrentqueue croncpp::croncpp glm::glm
            magic_enum::magic_enum re2::re2 spdlog::spdlog stduuid::stduuid uvw::uvw xbyak::xbyak)

    # MongoCXX
    #find_package(mongocxx REQUIRED)
    #find_package(bsoncxx REQUIRED)
    #find_package(mongoc-1.0 CONFIG REQUIRED)
    #include_directories(${LIBMONGOCXX_INCLUDE_DIR})
    #include_directories(${LIBBSONCXX_INCLUDE_DIR})
    include_directories(${SOURCE_PATH}/../mongo)

    target_link_libraries(${PROJ} PUBLIC "${SOURCE_PATH}/../mongo/bson-1.0.lib")
    target_link_libraries(${PROJ} PUBLIC "${SOURCE_PATH}/../mongo/mongoc-1.0.lib")
    target_link_libraries(${PROJ} PUBLIC "${SOURCE_PATH}/../mongo/bsoncxx.lib")
    target_link_libraries(${PROJ} PUBLIC "${SOURCE_PATH}/../mongo/mongocxx.lib")
    #target_link_libraries(${PROJ} PUBLIC mongo::mongoc_shared)
    #target_link_libraries(${PROJ} PUBLIC mongo::bsoncxx_shared)
    #target_link_libraries(${PROJ} PUBLIC mongo::mongocxx_shared)

    # FLCore

    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreCommon.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreDACom.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreDALib.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreFLServerEXE.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreRemoteClient.lib")
    target_link_libraries(${PROJ} PUBLIC "${SDK_PATH}/lib/FLCoreServer.lib")
endfunction()