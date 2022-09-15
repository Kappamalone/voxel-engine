SET(SOURCES
    stb_image.h
)

add_library(stb_image STATIC ${SOURCES})
target_include_directories(stb_image PUBLIC .)
set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE C)
