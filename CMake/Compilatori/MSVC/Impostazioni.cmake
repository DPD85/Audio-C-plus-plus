# ----- Impostazioni specifiche per MSVC -----

set(CMAKE_CONFIGURATION_TYPES Debug Release)

message(STATUS "MSVC: abilito avvisi di livello 3")
add_compile_options("/W3")

message(STATUS "MSVC: abilito compilazione multi processore")
add_compile_options("/MP")

message(STATUS "MSVC: abilito modalità di conformità")
add_compile_options("/permissive-")

message(STATUS "MSVC: abilito le funzioni intriseche")
add_compile_options("/Oi")

message(STATUS "MSVC: linker genera informazioni di debug in configurazione release")
add_link_options("/DEBUG")

# Configurazione globale di MSBuild:
#   * disabilita l'analisi statica del codice
configure_file(Directory.Build.props.in Directory.Build.props COPYONLY)
