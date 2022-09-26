#include "voxel_engine.h"

// Project Description:
// common/ -> common opengl abstractions and utility macros/functions
//
// TODO: cubemaps
// TODO: rule of 3/5 for all the classes
/*
chunks:
use element buffers for vertices
more efficient data types for voxels data
something about texture coordinates? (texture arrays)
greedy meshing

chunk manager:
frustum culling using stuff like octrees (octree not needed)
multithreaded voxel

chunk memory allocator?
ray intersections for block breaking?

TODOs:
how to generate structures like trees (done in chunk manager?)
water mesh generation
 */

int main() {
  auto voxel_engine = VoxelEngine(1400, 1000);
  voxel_engine.run();
}
