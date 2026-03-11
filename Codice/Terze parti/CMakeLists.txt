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
    set(PA_ENABLE_DEBUG_OUTPUT ON CACHE BOOL "" FORCE)
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

# ----- vk-bootstrap -----

CPMAddPackage(
    NAME vk-bootstrap
    VERSION 1.4.336
    GITHUB_REPOSITORY charles-lunarg/vk-bootstrap
)

# ----- ImGui -----

CPMAddPackage(
    NAME ImGui
    VERSION 1.92.6-docking
    GITHUB_REPOSITORY ocornut/imgui
    # Include il file "ImGui-Config.h" quando la macro IMGUI_USER_CONFIG è definita ignorando il contenuto della macro.
    PATCHES ImGui-Config.patch
    DOWNLOAD_ONLY ON
)

# Libreria principale

add_library(ImGui STATIC)

target_sources(ImGui
    PRIVATE
        ${ImGui_SOURCE_DIR}/imgui.cpp
        ${ImGui_SOURCE_DIR}/imgui.h
        ${ImGui_SOURCE_DIR}/imgui_demo.cpp
        ${ImGui_SOURCE_DIR}/imgui_draw.cpp
        ${ImGui_SOURCE_DIR}/imgui_internal.h
        ${ImGui_SOURCE_DIR}/imgui_tables.cpp
        ${ImGui_SOURCE_DIR}/imgui_widgets.cpp
        ${ImGui_SOURCE_DIR}/imstb_rectpack.h
        ${ImGui_SOURCE_DIR}/imstb_textedit.h
        ${ImGui_SOURCE_DIR}/imstb_truetype.h

        ${CMAKE_CURRENT_SOURCE_DIR}/ImGui-Config.h
)

target_compile_definitions(ImGui
    PUBLIC
        IMGUI_USER_CONFIG
)

target_include_directories(ImGui
    PUBLIC
        ${ImGui_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
    target_compile_options(ImGui PRIVATE "/utf-8")
endif()

make_source_group(TARGET ImGui ADDITIONAL_SOURCE_DIR ${ImGui_SOURCE_DIR})

# Backend SDL3

find_package(SDL3 REQUIRED)

add_library(ImGui-SDL3 STATIC)

target_sources(ImGui-SDL3
    PRIVATE
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
)

target_link_libraries(ImGui-SDL3
    PUBLIC
        ImGui
        SDL3::SDL3
)

target_include_directories(ImGui-SDL3 PUBLIC ${ImGui_SOURCE_DIR}/backends)

if(MSVC)
    target_compile_options(ImGui-SDL3 PRIVATE "/utf-8")
endif()

make_source_group(TARGET ImGui-SDL3 ADDITIONAL_SOURCE_DIR ${ImGui_SOURCE_DIR}/backends)

# Backend Vulkan

find_package(Vulkan REQUIRED)

add_library(ImGui-Vulkan STATIC)

target_sources(ImGui-Vulkan
    PRIVATE
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
)

target_link_libraries(ImGui-Vulkan
    PUBLIC
        ImGui
        Vulkan::Vulkan
)

target_include_directories(ImGui-Vulkan PUBLIC ${ImGui_SOURCE_DIR}/backends)

if(MSVC)
    target_compile_options(ImGui-Vulkan PRIVATE "/utf-8")
endif()

make_source_group(TARGET ImGui-Vulkan ADDITIONAL_SOURCE_DIR ${ImGui_SOURCE_DIR}/backends)

# FreeType

find_package(Freetype REQUIRED)
find_package(plutosvg REQUIRED)

add_library(ImGui-FreeType STATIC)

target_sources(ImGui-FreeType
    PRIVATE
        ${ImGui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
        ${ImGui_SOURCE_DIR}/misc/freetype/imgui_freetype.h
)

target_link_libraries(ImGui-FreeType
    PUBLIC
        ImGui
        Freetype::Freetype
        plutosvg::plutosvg
)

target_include_directories(ImGui-FreeType PUBLIC ${ImGui_SOURCE_DIR}/misc/freetype)

make_source_group(TARGET ImGui-FreeType ADDITIONAL_SOURCE_DIR ${ImGui_SOURCE_DIR}/misc/freetype)

# Aiuti debuggers

if(MSVC)
    add_library(ImGui-Debuggers INTERFACE)

    target_sources(ImGui-Debuggers
        INTERFACE
            ${ImGui_SOURCE_DIR}/misc/debuggers/imgui.natstepfilter
            ${ImGui_SOURCE_DIR}/misc/debuggers/imgui.natvis
    )

    make_source_group(TARGET ImGui-Debuggers ADDITIONAL_SOURCE_DIR ${ImGui_SOURCE_DIR}/misc/debuggers)
endif()


# ----- ImPlot -----

CPMAddPackage(
    NAME ImPlot
    VERSION 0.17
    GITHUB_REPOSITORY epezent/implot
    DOWNLOAD_ONLY ON
)

add_library(ImPlot STATIC)

target_sources(ImPlot
    PRIVATE
        ${ImPlot_SOURCE_DIR}/implot.cpp
        ${ImPlot_SOURCE_DIR}/implot.h
        ${ImPlot_SOURCE_DIR}/implot_demo.cpp
        ${ImPlot_SOURCE_DIR}/implot_internal.h
        ${ImPlot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(ImPlot PUBLIC ${ImPlot_SOURCE_DIR})

if(MSVC)
    target_compile_options(ImPlot PRIVATE "/utf-8")
endif()

target_link_libraries(ImPlot PUBLIC ImGui)

make_source_group(TARGET ImPlot ADDITIONAL_SOURCE_DIR ${ImPlot_SOURCE_DIR})

# ----- IconFontCppHeaders -----

add_library(IconFontCppHeaders INTERFACE)

target_sources(IconFontCppHeaders
    INTERFACE
        IconFontCppHeaders/IconsFontAwesome7.h
)

target_include_directories(IconFontCppHeaders
    INTERFACE
        IconFontCppHeaders
)
