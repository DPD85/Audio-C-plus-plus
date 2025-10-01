# ----- Impostazioni specifiche per Linux -----

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
        message(FATAL_ERROR "Il compilatore C/C++ ${CMAKE_CXX_COMPILER_ID} non è supportato")
    else()
        message(FATAL_ERROR "I compilatori C ${CMAKE_C_COMPILER_ID} e C++ ${CMAKE_CXX_COMPILER_ID} non sono supportati")
    endif()
endif()

include(CMake/Compilatori/GCC/Impostazioni.cmake)
include(CMake/Compilatori/GCC/Funzioni.cmake)
