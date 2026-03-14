# ----- Funzioni specifiche per MSVC -----

# Organize the sources of a target in various source groups corresponding to file folders
# Arguments:
#   TARGET                            The target name
#   REMOVE_PREFIX         [opzionale] File path prefix to remove when the source groups are created
#   ADDITIONAL_SOURCE_DIR [opzionale] Direttorio dei file sorgenti addizionale a quello predefinito
#                                     (CMAKE_CURRENT_SOURCE_DIR)
function(make_source_group)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;REMOVE_PREFIX;ADDITIONAL_SOURCE_DIR"
        ""
        ${ARGN}
    )
    if(NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "TARGET argument not specified")
    endif()

    string(REPLACE "+" "\\+" bin_dir "${CMAKE_CURRENT_BINARY_DIR}")
    string(REPLACE "+" "\\+" remove_prefix "${ARG_REMOVE_PREFIX}")
    string(REPLACE "+" "\\+" additional_src_dir "${ARG_ADDITIONAL_SOURCE_DIR}")

    get_target_property(sources ${ARG_TARGET} SOURCES)

    foreach(file_path ${sources})
        if(ARG_ADDITIONAL_SOURCE_DIR AND file_path MATCHES "^${additional_src_dir}")
            # File sorgente da un direttorio diverso dal default (CMAKE_CURRENT_SOURCE_DIR)
            source_group(
                TREE ${ARG_ADDITIONAL_SOURCE_DIR}/${ARG_REMOVE_PREFIX}
                FILES ${file_path}
            )
        elseif(file_path MATCHES "^${bin_dir}")
            # Generated sources code
            source_group(
                TREE ${CMAKE_CURRENT_BINARY_DIR}
                FILES ${file_path}
            )
        elseif(NOT file_path MATCHES "^${remove_prefix}")
            # If source file don't begin with the specified prefix don't group it
        else()
            source_group(
                TREE ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_REMOVE_PREFIX}
                FILES ${file_path}
            )
        endif()
    endforeach()
endfunction()
