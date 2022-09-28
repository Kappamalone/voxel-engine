SET(SOURCES
    main.cpp
    voxel_engine.h
    voxel_engine.cpp
    chunk.h
    chunk.cpp
    player_camera.h
    player_camera.cpp
    chunk_manager.h
    chunk_manager.cpp
    frustum.h
    frustum.cpp
    lerp_points.h
)

add_subdirectory(common)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE common stb_image glm perlin_noise)
# target_link_libraries(${PROJECT_NAME} PRIVATE common glfw glad imgui fmt stb_image glm)
# target_include_directories(${PROJECT_NAME} PRIVATE ../externals/stb_image/)

