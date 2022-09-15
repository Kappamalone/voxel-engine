#pragma once
#include "player_camera.h"
#include "window.h"

class VoxelEngine {
private:
  Window window;
  PlayerCamera player_camera;

  // frame time variables
  double delta_time = 0.0f;
  double current_frame = 0.0f;
  double last_frame = 0.0f;

public:
  VoxelEngine(int viewport_width, int viewport_height);

  void run();
  void handle_input();
};
