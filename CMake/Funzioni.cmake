# Configura la generazione del codice per incorporare un file binario all'interno di un programma o libreria.
# Parametri:
#   INPUT_FILE    il file che deve essere incorporato;
#   OUTPUT_NAME   il nome dei file sorgenti generati (.h e .cpp);
#   NAMESPACE     il nome del name space dentro il quale inserire tutto il codice generato;
#   SYMBOL_NAME   il nome del name space dentro cui inserire il codice relativo al file incorporato;
#   COMMENT       [opzionale] il commento da stampare subito prima di generare il codice;
#   NO_COMPRESS   [opzionale] se specificato evita che il file d'input venga compresso prima di venire incorporato.
function(IncorporazioneFile)
    cmake_parse_arguments(
        ARG
        "NO_COMPRESS"
        "INPUT_FILE;OUTPUT_NAME;NAMESPACE;SYMBOL_NAME;COMMENT"
        ""
        ${ARGN}
    )
    if(NOT DEFINED ARG_INPUT_FILE)
        message(FATAL_ERROR "INPUT_FILE argument not specified")
    endif()
    if(NOT DEFINED ARG_OUTPUT_NAME)
        message(FATAL_ERROR "OUTPUT_NAME argument not specified")
    endif()
    if(NOT DEFINED ARG_NAMESPACE)
        message(FATAL_ERROR "NAMESPACE argument not specified")
    endif()
    if(NOT DEFINED ARG_SYMBOL_NAME)
        message(FATAL_ERROR "SYMBOL_NAME argument not specified")
    endif()

    if(ARG_NO_COMPRESS)
        set(parametriAggiuntivi "-nocompress")
    else()
        set(parametriAggiuntivi "")
    endif()

    add_custom_command(
        OUTPUT
            "${ARG_OUTPUT_NAME}.cpp"
            "${ARG_OUTPUT_NAME}.h"
        COMMAND $<TARGET_FILE:BinaryToCompressedC> ${parametriAggiuntivi} -nostatic "${ARG_INPUT_FILE}" ${ARG_NAMESPACE} ${ARG_SYMBOL_NAME} "${ARG_OUTPUT_NAME}"
        MAIN_DEPENDENCY "${ARG_INPUT_FILE}"
        DEPENDS BinaryToCompressedC
        COMMENT "${ARG_COMMENT}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
endfunction()
