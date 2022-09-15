#include "voxel_engine.h"
// Project Description:
// common/ -> common opengl abstractions and utility macros/functions

int main() {
  auto voxel_engine = VoxelEngine(1400, 1000);
  voxel_engine.run();
}
