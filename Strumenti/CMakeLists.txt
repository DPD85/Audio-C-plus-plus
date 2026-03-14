set(CMAKE_FOLDER "Strumenti")

# ----- BinaryToCompressedC -----

add_executable(BinaryToCompressedC)

target_sources(BinaryToCompressedC
    PRIVATE
        BinaryToCompressedC.cpp
)

make_source_group(TARGET BinaryToCompressedC)
