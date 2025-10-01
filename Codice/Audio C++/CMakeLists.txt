find_package(Boost REQUIRED COMPONENTS container)

add_executable(Audio-C-plus-plus)

set_target_properties(Audio-C-plus-plus
    PROPERTIES
        OUTPUT_NAME "Audio C++"
)

target_sources(Audio-C-plus-plus
    PRIVATE
        "Audio C++.cpp"
        Clock.h
        CodaEliminazione.cpp
        CodaEliminazione.h
        CostantiEdAltro.h
        Filtri.h
        IntestazionePrecompilata.h
        Inviluppo.h
        Normalizzatore.h
        Oscillatori.h
        StrumentoMusicale.h
        Volume.h
        WAVE.cpp
        WAVE.h
)

target_precompile_headers(Audio-C-plus-plus
    PRIVATE
        IntestazionePrecompilata.h
)

target_compile_definitions(Audio-C-plus-plus
    PRIVATE
        _UNICODE
)

target_link_libraries(Audio-C-plus-plus
    PRIVATE
        Winmm.lib
        Boost::container
)

make_source_group(TARGET Audio-C-plus-plus)
