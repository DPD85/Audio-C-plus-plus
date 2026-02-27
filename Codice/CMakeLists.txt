CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

# ----- Set-up progetto e linguaggi -----

project("Audio C++" LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disabilito la scansione dei sorgenti per trovare dipendenze dai moduli (i moduli sono una funzionalità presente dal
# C++20 in poi), poiché il generatore Ninja ha un bug relativo alle cartelle contenenti spazi.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ----- Versione del tool set MSVC -----

if(MSVC)
    if(NOT CMAKE_GENERATOR_TOOLSET)
        message(FATAL_ERROR "La versione del tool set di MSVC deve essere indicata esplicitamente")
    endif()
    message(STATUS "MSVC: tool set ${CMAKE_GENERATOR_TOOLSET}")
endif()

# ----- Inclusion of modules -----

include(CMake/Piattaforma.cmake)
include(CMake/Conan.cmake)
include(CMake/Funzioni.cmake)
include(CMake/Impostazioni.cmake)

# ----- IDE configuration -----

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Audio-C-plus-plus)

# ----- Source code -----

add_subdirectory("Terze parti")
add_subdirectory("Audio C++")

include_external_msproject(Grafici ${CMAKE_CURRENT_SOURCE_DIR}/Grafici/Grafici.csproj PLATFORM AnyCPU)

# ----- Solution Items -----

add_custom_target(SolutionItems SOURCES
    CMake/Compilatori/GCC/Funzioni.cmake
    CMake/Compilatori/GCC/Impostazioni.cmake
    CMake/Compilatori/MSVC/Funzioni.cmake
    CMake/Compilatori/MSVC/Impostazioni.cmake
    CMake/Conan.cmake
    CMake/Funzioni.cmake
    CMake/Impostazioni.cmake
    CMake/Piattaforma.cmake
    CMake/Piattaforme/Linux/Impostazioni.cmake
    CMake/Piattaforme/Windows/Impostazioni.cmake

    "Profili Conan/Linux GCC 12 x64 debug"
    "Profili Conan/Linux GCC 12 x64 release"
    "Profili Conan/Windows v143 x64 debug"
    "Profili Conan/Windows v143 x64 release"

    "Terze parti/CMakeLists.txt"
    "Terze parti/Readme.md"

    .clang-format
    .editorconfig
    .gitignore
    CMakeLists.txt
    CMakePresets.json
    conanfile.py
    Directory.Build.props.in
    Readme.md
)

make_source_group(TARGET SolutionItems)
