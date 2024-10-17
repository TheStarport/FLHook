if (NOT EXISTS "${PROJECT_SOURCE_DIR}/build.conf")
    message(STATUS "build.conf missing, generating new one...")
    file(APPEND "${PROJECT_SOURCE_DIR}/build.conf" "#BINARY_COPY_DESTINATION=C:\\Path\\To\\Freelancer\\EXE")
else()
    message(STATUS "Reading CMake variables from build.conf")
    file(STRINGS "${PROJECT_SOURCE_DIR}/build.conf" CONFIG_CONTENTS)
    foreach(NAME_AND_VALUE ${CONFIG_CONTENTS})
        string(REGEX REPLACE "#.*" "" TRIMMED_NAME_AND_VALUE ${NAME_AND_VALUE})
        if ("${TRIMMED_NAME_AND_VALUE}" STREQUAL "")
            continue()
        endif()

        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" TRIMMED_NAME_AND_VALUE ${TRIMMED_NAME_AND_VALUE})
        if ("${TRIMMED_NAME_AND_VALUE}" STREQUAL "")
            continue()
        endif()

        # Find variable name
        string(REGEX MATCH "^[^=]+" V_NAME ${TRIMMED_NAME_AND_VALUE})
        # Find the value
        string(REPLACE "${V_NAME}=" "" V_VALUE ${TRIMMED_NAME_AND_VALUE})

        if ("${V_NAME}" STREQUAL "" OR "${V_VALUE}" STREQUAL "")
            continue()
        endif()

        # Set the variable
        message(STATUS "Setting \"${V_NAME}\" = \"${V_VALUE}\"")
        set(${V_NAME} "${V_VALUE}")
    endforeach()
endif()