set(CMAKE_FOLDER "Terze parti")

# ----- PortAudio -----

set(PA_BUILD_SHARED    OFF CACHE BOOL "" FORCE)
set(PA_BUILD_STATIC    ON  CACHE BOOL "" FORCE)
set(PA_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(PA_DISABLE_INSTALL ON  CACHE BOOL "" FORCE)

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
endif()

add_subdirectory(PortAudio)

if(UNIX)
    # I seguenti avvisi sono trattanti come avvisi e non come errori.
    target_compile_options(portaudio_static
        PRIVATE
            -Wno-error=implicit-function-declaration
    )
endif()
