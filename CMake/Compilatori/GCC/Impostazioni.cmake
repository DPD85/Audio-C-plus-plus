# ----- Impostazioni specifiche per GCC -----

message(STATUS "GCC: abilito posizione indipendente del codice")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
