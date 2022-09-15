SET(SOURCES
    window.h
    window.cpp
    shader_program.h
    shader_program.cpp
    color.h
    common.h
)

find_package(glfw3 REQUIRED)

add_library(common STATIC ${SOURCES})
target_link_libraries(common PUBLIC glfw glad imgui fmt)
target_include_directories(common PUBLIC .)
