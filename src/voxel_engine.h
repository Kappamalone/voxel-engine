#pragma once
#include "chunk_manager.h"
#include "player_camera.h"
#include "window.h"

class VoxelEngine {
private:
  Window window;
  PlayerCamera player_camera;
  ChunkManager chunk_manager;

  bool show_wireframe = false;

  // frame time variables
  double delta_time = 0.0f;
  double current_frame = 0.0f;
  double last_frame = 0.0f;

public:
  VoxelEngine(int viewport_width, int viewport_height);

  void run();
  void handle_input();
  void toggle_wireframe() {
    static constexpr uint32_t map[2] = {GL_FILL, GL_LINE};
    show_wireframe = !show_wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, map[show_wireframe]);
  }
};
