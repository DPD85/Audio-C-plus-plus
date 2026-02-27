find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale)
find_package(platform_folders REQUIRED)
find_package(nlohmann_json REQUIRED)

# Creazione delle cartelle di output per i file generati poiché non tutti i generatori di CMake la creano automaticamente.
# Per esempio Ninja le crea mentre Unix Makefiles non lo fa.
make_directory("${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts")
make_directory("${CMAKE_CURRENT_BINARY_DIR}/Generated/Lingue")

# IBM Plex Sans.ttf
IncorporazioneFile(
    INPUT_FILE "${CMAKE_SOURCE_DIR}/Terze parti/Fonts/IBM Plex Sans/IBM Plex Sans.ttf"
    OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSans"
    NAMESPACE Fonts
    SYMBOL_NAME IBMPlexSans
    COMMENT "Incorporazione di IBM Plex Sans.ttf"
)

# IBM Plex Sans italico.ttf
IncorporazioneFile(
    INPUT_FILE "${CMAKE_SOURCE_DIR}/Terze parti/Fonts/IBM Plex Sans/IBM Plex Sans italico.ttf"
    OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSansItalic"
    NAMESPACE Fonts
    SYMBOL_NAME IBMPlexSansItalic
    COMMENT "Incorporazione di IBM Plex Sans italico.ttf"
)

# BabelStone Flags PUA.ttf
IncorporazioneFile(
    INPUT_FILE "${CMAKE_SOURCE_DIR}/Terze parti/Fonts/BabelStone Flags/BabelStone Flags PUA.ttf"
    OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/BabelStoneFlags"
    NAMESPACE Fonts
    SYMBOL_NAME BabelStoneFlags
    COMMENT "Incorporazione di BabelStone Flags PUA.ttf"
)

# Font Awesome 7 Free-Solid-900.otf
IncorporazioneFile(
    INPUT_FILE "${CMAKE_SOURCE_DIR}/Terze parti/Fonts/Font Awesome/Font Awesome 7 Free-Solid-900.otf"
    OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/AwesomeSolid"
    NAMESPACE Fonts
    SYMBOL_NAME AwesomeSolid
    COMMENT "Incorporazione di Font Awesome 7 Free-Solid-900.otf"
)

# Lingua.mo
IncorporazioneFile(
    INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Lingue/en_GB/LC_MESSAGES/Lingua.mo"
    OUTPUT_NAME "${CMAKE_CURRENT_BINARY_DIR}/Generated/Lingue/en_GB"
    NAMESPACE CatalogoLingue
    SYMBOL_NAME en_GB
    NO_COMPRESS
    COMMENT "Incorporazione di Lingua.mo per la lingua en_GB"
)

add_executable(Germen)

target_sources(Germen
    PRIVATE
        "Clock.h"
        "CodaCancellazione.cpp"
        "CodaCancellazione.h"
        "CostantiEdAltro.h"
        "Disegnatore.cpp"
        "Disegnatore.h"
        "GUI.cpp"
        "Icone bandiere.h"
        "Impostazioni.cpp"
        "Impostazioni.h"
        "Internazionalizzazione.cpp"
        "Internazionalizzazione.h"
        "IntestazionePrecompilata.h"
        "Main.cpp"
        "Temi.cpp"
        "TestiGUI.h"

        "Estensioni ImGui/Barra di stato.cpp"
        "Estensioni ImGui/Estensioni.h"

        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/AwesomeSolid.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/AwesomeSolid.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/BabelStoneFlags.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/BabelStoneFlags.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSans.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSans.h"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSansItalic.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Fonts/IBMPlexSansItalic.h"

        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Lingue/en_GB.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated/Lingue/en_GB.h"
)

target_precompile_headers(Germen
    PRIVATE
        IntestazionePrecompilata.h
)

target_include_directories(Germen
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/Generated"
)

target_link_libraries(Germen
    PRIVATE
        Boost::locale
        IconFontCppHeaders
        ImGui
        ImGui-FreeType
        ImGui-SDL3
        ImGui-Vulkan
        ImPlot
        nlohmann_json::nlohmann_json
        sago::platform_folders
        SDL3::SDL3
        vk-bootstrap::vk-bootstrap
        Vulkan::Vulkan
)

if(MSVC)
    target_link_libraries(Germen
        PRIVATE
            ImGui-Debuggers
    )

    # Alcune dipendenze di Freetype sono prive dei file PDB.
    # LNK4099: PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info
    target_link_options(Germen
        PRIVATE
            "/ignore:4099"
    )
endif()

make_source_group(TARGET Germen ADDITIONAL_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
