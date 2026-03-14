# ----- Impostazioni specifiche per Windows -----

if(MSVC)
    include(CMake/Compilatori/MSVC/Funzioni.cmake)
    include(CMake/Compilatori/MSVC/Impostazioni.cmake)
else()
    if(CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
        message(FATAL_ERROR "Il compilatore C/C++ ${CMAKE_CXX_COMPILER_ID} non è supportato.")
    else()
        message(FATAL_ERROR "I compilatori C ${CMAKE_C_COMPILER_ID} e C++ ${CMAKE_CXX_COMPILER_ID} non sono supportati.")
    endif()
endif()
