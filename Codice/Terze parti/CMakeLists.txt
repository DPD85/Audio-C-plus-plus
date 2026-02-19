set(CMAKE_FOLDER "Terze parti")

# ----- PortAudio -----

set(PA_BUILD_SHARED    OFF CACHE BOOL "" FORCE)
set(PA_BUILD_STATIC    ON  CACHE BOOL "" FORCE)
set(PA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(PA_DISABLE_INSTALL ON  CACHE BOOL "" FORCE)

# Abilita il log di debug di PortAudio
#set(PA_ENABLE_DEBUG_OUTPUT ON CACHE BOOL "" FORCE)

if(WIN32)
    # Su Windows voglio utilizzare solo WASAPI
    set(PA_USE_WASAPI            ON  CACHE BOOL "" FORCE)
    set(PA_USE_ASIO              OFF CACHE BOOL "" FORCE)
    set(PA_USE_DS                OFF CACHE BOOL "" FORCE)
    set(PA_USE_WDMKS             OFF CACHE BOOL "" FORCE)
    set(PA_USE_WDMKS_DEVICE_INFO OFF CACHE BOOL "" FORCE)
    set(PA_USE_WMME              OFF CACHE BOOL "" FORCE)
    set(PA_USE_DIRECTSOUNDFULLDUPLEXCREATE OFF CACHE BOOL "" FORCE)
elseif(LINUX)
    # Su Linux lascio disponibili tutti i back-end disponibili (ALSA e JACK)
    # Nota: se PortAudio non trova il SDK di un back-end esso sarà disattivato in fase di compilazione.
endif()

block()
    # PortAudio ha problemi a compilare correttamente con impostazioni diverse dal default, in particolar modo
    # disattivando le estensioni del linguaggio C.
    unset(CMAKE_C_STANDARD)
    unset(CMAKE_C_STANDARD_REQUIRED)
    unset(CMAKE_C_EXTENSIONS)

    unset(CMAKE_CXX_STANDARD)
    unset(CMAKE_CXX_STANDARD_REQUIRED)
    unset(CMAKE_CXX_EXTENSIONS)

    add_subdirectory(PortAudio)

    if(UNIX)
        # Disabilito avvisi specifici generati da PortAudio.
        target_compile_options(portaudio_static
            PRIVATE
                -Wno-deprecated-declarations
        )
    endif()
endblock()

if(UNIX AND NOT PA_USE_ALSA AND NOT PA_USE_JACK)
    message(FATAL_ERROR "PortAudio non ha trovato ne il SDK di ALSA ne quello di Jack, almeno uno deve essere disponibile.")
endif()
