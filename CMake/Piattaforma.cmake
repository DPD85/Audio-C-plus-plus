# Rileva su quale piattaforma stiamo compilando ed include il file di impostazioni corrispondente

if(NOT "${CMAKE_SIZEOF_VOID_P}" MATCHES 8)
    message(FATAL_ERROR "La compilazione a 32 bit non è supportata")
endif()

if(${CMAKE_C_COMPILER_ID} STREQUAL ${CMAKE_CXX_COMPILER_ID})
    message(STATUS "Compilazione su sistema ${CMAKE_SYSTEM_NAME} con compilatore C/C++ ${CMAKE_CXX_COMPILER_ID}")
else()
    message(STATUS "Compilazione su sistema ${CMAKE_SYSTEM_NAME} con compilatori C ${CMAKE_C_COMPILER_ID} e C++ ${CMAKE_CXX_COMPILER_ID}")
endif()

if(WIN32)
    include(CMake/Piattaforme/Windows/Impostazioni.cmake)
elseif(LINUX)
    include(CMake/Piattaforme/Linux/Impostazioni.cmake)
else()
    message(FATAL_ERROR "La piattaforma ${CMAKE_SYSTEM_NAME} non è supportata")
endif()
