# ----- Impostazioni specifiche per Windows -----

if(MSVC)
    include(CMake/Compilatori/MSVC/Impostazioni.cmake)
    include(CMake/Compilatori/MSVC/Funzioni.cmake)
else()
    message(FATAL_ERROR "I compilatori C ${CMAKE_C_COMPILER_ID} e C++ ${CMAKE_CXX_COMPILER_ID} non sono supportati")
endif()
