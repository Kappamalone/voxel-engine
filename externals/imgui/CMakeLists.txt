SET(SOURCES
    imconfig.h
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_glfw.cpp
    imgui_impl_glfw.h
    imgui_impl_opengl3.cpp
    imgui_impl_opengl3.h
    imgui_impl_opengl3_loader.h
    # what do these files do?
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
)

add_library(imgui STATIC ${SOURCES})
target_include_directories(imgui PUBLIC .)
