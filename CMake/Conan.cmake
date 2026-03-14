# ----- Cerco Conan nel sistema -----

if (WIN32)
    find_program(CONAN_PATH
        NAMES conan
        REQUIRED
    )
elseif(LINUX)
    find_program(CONAN_PATH
        NAMES conan
        HINTS /home/user/.local/bin
        REQUIRED
)
endif()

# ----- Profilo di Conan -----

set(CONAN_PROFILE_DIR "${CMAKE_SOURCE_DIR}/Profili Conan")

if (WIN32)
    set(CONAN_PROFILE_NAME              "Windows ${CMAKE_GENERATOR_TOOLSET} x64")
    set(CONAN_PROFILE_DEBUG             "${CONAN_PROFILE_DIR}/${CONAN_PROFILE_NAME} debug")
    set(CONAN_PROFILE_RELEASE           "${CONAN_PROFILE_DIR}/${CONAN_PROFILE_NAME} release")
    set(CONAN_PROFILE_DEBUG_TIMESTAMP   "${CMAKE_BINARY_DIR}/${CONAN_PROFILE_NAME} debug.timestamp")
    set(CONAN_PROFILE_RELEASE_TIMESTAMP "${CMAKE_BINARY_DIR}/${CONAN_PROFILE_NAME} release.timestamp")

    message(STATUS "Profilo Conan (Debug)  : ${CONAN_PROFILE_DEBUG}")

    if(NOT EXISTS ${CONAN_PROFILE_DEBUG})
        message(FATAL_ERROR "Il profilo Conan di debug non esiste.")
    endif()

    message(STATUS "Profilo Conan (Release): ${CONAN_PROFILE_RELEASE}")

    if(NOT EXISTS ${CONAN_PROFILE_RELEASE})
        message(FATAL_ERROR "Il profilo Conan di release non esiste.")
    endif()
elseif(LINUX)
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
    set(CONAN_PROFILE_NAME      "Linux GCC ${GCC_MAJOR_VERSION} x64 ${CMAKE_BUILD_TYPE_LOWER}")
    set(CONAN_PROFILE           "${CONAN_PROFILE_DIR}/${CONAN_PROFILE_NAME}")
    set(CONAN_PROFILE_TIMESTAMP "${CMAKE_BINARY_DIR}/${CONAN_PROFILE_NAME}.timestamp")

    message(STATUS "Profilo Conan: ${CONAN_PROFILE}")

    if(NOT EXISTS ${CONAN_PROFILE})
        message(FATAL_ERROR "Il profilo Conan non esiste.")
    endif()
endif()

# ----- Installazione dipendenze (invocazione Conan) -----

set(CONAN_INPUT_DIR  "${CMAKE_SOURCE_DIR}")
set(CONAN_OUTPUT_DIR "${CMAKE_BINARY_DIR}")

set(CONAN_FILE           "${CMAKE_SOURCE_DIR}/conanfile.py")
set(CONAN_FILE_TIMESTAMP "${CMAKE_BINARY_DIR}/conanfile.py.timestamp")

# ----- Controllo se i file di configurazione di Conan sono cambiati

if(${CONAN_FILE} IS_NEWER_THAN ${CONAN_FILE_TIMESTAMP})
    set(ESEGUIRE_CONAN YES)
endif()

if(WIN32)
    if(ESEGUIRE_CONAN OR ${CONAN_PROFILE_DEBUG} IS_NEWER_THAN ${CONAN_PROFILE_DEBUG_TIMESTAMP})
        set(ESEGUIRE_CONAN_PROFILE_DEBUG YES)
    endif()
    if(ESEGUIRE_CONAN OR ${CONAN_PROFILE_RELEASE} IS_NEWER_THAN ${CONAN_PROFILE_RELEASE_TIMESTAMP})
        set(ESEGUIRE_CONAN_PROFILE_RELEASE YES)
    endif()
elseif(LINUX)
    if(ESEGUIRE_CONAN OR ${CONAN_PROFILE} IS_NEWER_THAN ${CONAN_PROFILE_TIMESTAMP})
        set(ESEGUIRE_CONAN_PROFILE YES)
    endif()
endif()

# ----- Esegue Conan se necessario

if(WIN32)
    # ----- Debug

    if(ESEGUIRE_CONAN_PROFILE_DEBUG)
        message(STATUS "Installazione delle dipendenze in debug tramite Conan...")

        execute_process(
            COMMAND ${CONAN_PATH} install "${CONAN_INPUT_DIR}" -pr:a "${CONAN_PROFILE_DEBUG}" -of "${CONAN_OUTPUT_DIR}" --build=missing
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )

        file(TOUCH "${CONAN_PROFILE_DEBUG_TIMESTAMP}")
    endif()

    # ----- Release

    if(ESEGUIRE_CONAN_PROFILE_RELEASE)
        message(STATUS "Installazione delle dipendenze in release tramite Conan...")

        execute_process(
            COMMAND ${CONAN_PATH} install "${CONAN_INPUT_DIR}" -pr:a "${CONAN_PROFILE_RELEASE}" -of "${CONAN_OUTPUT_DIR}" --build=missing
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )

        file(TOUCH "${CONAN_PROFILE_RELEASE_TIMESTAMP}")
    endif()
elseif(LINUX)
    if(ESEGUIRE_CONAN_PROFILE)
        message(STATUS "Installazione delle dipendenze tramite Conan...")

        execute_process(
            COMMAND ${CONAN_PATH} install "${CONAN_INPUT_DIR}" -pr:a "${CONAN_PROFILE}" -of "${CONAN_OUTPUT_DIR}" --build=missing
            COMMAND_ECHO STDOUT
            COMMAND_ERROR_IS_FATAL ANY
        )

        file(TOUCH "${CONAN_PROFILE_TIMESTAMP}")
    endif()
endif()

# -----

if(ESEGUIRE_CONAN)
    file(TOUCH ${CONAN_FILE_TIMESTAMP})
endif()

# ----- Configurazione percorsi di ricerca usati per trovare i pacchetti -----

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CONAN_OUTPUT_DIR}/conan")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CONAN_OUTPUT_DIR}/conan")
