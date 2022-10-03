#include "chunk.h"
#include "common.h"
#include "lerp_points.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

Chunk::Chunk(std::unordered_map<ChunkPos, Chunk>& world_chunks, int xoffset,
             int zoffset, siv::PerlinNoise& perlin_noise)
    : world_chunks(world_chunks), xoffset(xoffset), zoffset(zoffset),
      perlin_noise(perlin_noise) {
  create_voxels();
  bounding_box =
      BoundingBox{.min = glm::vec3(xoffset, 0, zoffset),
                  .max = glm::vec3(xoffset + CHUNK_WIDTH, CHUNK_HEIGHT,
                                   zoffset + CHUNK_DEPTH)};
}

void Chunk::create_voxels() {
  voxels.resize(CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT);

  LerpPoints lerp_points(Point(-1.0f, 60), Point(1.0f, 120));

  for (auto z = 0; z < CHUNK_DEPTH; z++) {
    for (auto x = 0; x < CHUNK_WIDTH; x++) {
      static constexpr int WATER_THRESHOLD = 90;
      static constexpr float PERLIN_SCALE = 0.035f;
      static constexpr float PERLIN_OCTAVES = 3;
      static constexpr float PERLIN_PERSISTENCE = 0.5;

      double perlin_value = (perlin_noise.octave2D_11(
          (xoffset + x) * PERLIN_SCALE, (zoffset - z) * PERLIN_SCALE,
          PERLIN_OCTAVES, PERLIN_PERSISTENCE));
      int height = lerp_points.interpolate(perlin_value);

      for (auto y = 0; y < height; y++) {
        /*
        // TODO: figure out how squishification works
        static constexpr double DENSITY_THRESHOLD = 0.35f;
        double density = perlin_noise.octave3D_11(
            (xoffset + x) * PERLIN_SCALE, y * PERLIN_SCALE,
            (zoffset - z) * PERLIN_SCALE, PERLIN_OCTAVES, PERLIN_PERSISTENCE);
        if (density >= DENSITY_THRESHOLD) {
          set_voxel(x, y, z, VoxelType::GRASS);
        }
        */
        if (y <= WATER_THRESHOLD) {
          set_voxel(x, y, z, VoxelType::DIRT);
        } else if (y >= height - 1) {
          set_voxel(x, y, z, VoxelType::GRASS);
        } else if (y >= height - 5) {
          set_voxel(x, y, z, VoxelType::DIRT);
        } else {
          set_voxel(x, y, z, VoxelType::STONE);
        }
      }
    }
  }
}

// TODO: maybe unspagettify the mesh creation a little *sob*

// TODO: use an enum here
void Chunk::emit_vertex_coordinates(int index, float x, float y, float z) {
  switch (index) {
    case 0:
      vertices_buffer.push_back(xoffset + x);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(zoffset - z);
      break;
    case 1:
      vertices_buffer.push_back(xoffset + x);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(zoffset - z - 1.0f);
      break;
    case 2:
      vertices_buffer.push_back(xoffset + x + 1.0f);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(zoffset - z - 1.0f);
      break;
    case 3:
      vertices_buffer.push_back(xoffset + x + 1.0f);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(zoffset - z);
      break;
    case 4:
      vertices_buffer.push_back(xoffset + x);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(zoffset - z);
      break;
    case 5:
      vertices_buffer.push_back(xoffset + x);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(zoffset - z - 1.0f);
      break;
    case 6:
      vertices_buffer.push_back(xoffset + x + 1.0f);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(zoffset - z - 1.0f);
      break;
    case 7:
      vertices_buffer.push_back(xoffset + x + 1.0f);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(zoffset - z);
      break;
    default:
      PANIC("what?\n");
      break;
  }
}

void Chunk::emit_texture_coordinates(TexturePosition position,
                                     int atlas_index) {
  static constexpr const int tex_atlas_rows = 16;
  int column = atlas_index % tex_atlas_rows;
  int row = atlas_index / tex_atlas_rows;
  float xoff = (float)column / (float)tex_atlas_rows;
  float yoff = (float)row / (float)tex_atlas_rows;

  // NOTE: tex coords are upside down to account for uv coords starting at
  // bottom left
  switch (position) {
    case TexturePosition::BOTTOM_LEFT:
      vertices_buffer.push_back(0.f + xoff);
      vertices_buffer.push_back(1.f / (float)tex_atlas_rows + yoff);
      break;
    case TexturePosition::BOTTOM_RIGHT:
      vertices_buffer.push_back(1.f / (float)tex_atlas_rows + xoff);
      vertices_buffer.push_back(1.f / (float)tex_atlas_rows + yoff);
      break;
    case TexturePosition::TOP_LEFT:
      vertices_buffer.push_back(0.f + xoff);
      vertices_buffer.push_back(0.f + yoff);
      break;
    case TexturePosition::TOP_RIGHT:
      vertices_buffer.push_back(1.f / (float)tex_atlas_rows + xoff);
      vertices_buffer.push_back(0.f + yoff);
      break;
  }
}

void Chunk::construct_face(BlockFaces face, int atlas_index, float x, float y,
                           float z) {
  switch ((BlockFaces)face) {
    case BlockFaces::BOTTOM:
      emit_vertex_coordinates(2, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(3, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(0, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(0, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(1, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(2, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::TOP:
      emit_vertex_coordinates(7, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(6, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(5, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(5, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(4, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(7, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::LEFT:
      emit_vertex_coordinates(0, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(4, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(5, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(5, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(1, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(0, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::RIGHT:
      emit_vertex_coordinates(2, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(6, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(7, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(7, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(3, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(2, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::FRONT:
      emit_vertex_coordinates(3, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(7, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(4, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(4, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(0, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(3, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::BACK:
      emit_vertex_coordinates(1, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      emit_vertex_coordinates(5, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      emit_vertex_coordinates(6, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      emit_vertex_coordinates(6, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      emit_vertex_coordinates(2, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      emit_vertex_coordinates(1, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
  }
}

void Chunk::create_mesh() {

  // algorithm:
  //  for each voxel that isn't an air type, check if any of it's six faces
  //  borders an air block, if so add that face to the mesh, else ignore
  if (mesh_created == true) {
    PRINT("Threading is hard...\n");
    return;
  }
  mesh_created = true;

  for (auto y = 0; y < CHUNK_HEIGHT; y++) {
    for (auto z = 0; z < CHUNK_DEPTH; z++) {
      for (auto x = 0; x < CHUNK_WIDTH; x++) {
        // check if current voxel isn't an air block
        auto voxel_type = get_voxel(x, y, z).voxel_type;
        if (voxel_type == VoxelType::AIR) {
          continue;
        }

        std::unordered_map<BlockFaces, int> tex_atlas_map =
            block_to_faces_map[voxel_type];

        for (auto face = (int)BlockFaces::BOTTOM; face < 6; face++) {
          switch ((BlockFaces)face) {
            case BlockFaces::BOTTOM: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (y == 0) {
                construct_face(BlockFaces::BOTTOM, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x, y - 1, z)) {
                construct_face(BlockFaces::BOTTOM, tex_atlas_index, x, y, z);
              }
              break;
            }
            case BlockFaces::TOP: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (y == CHUNK_HEIGHT - 1) {
                construct_face(BlockFaces::TOP, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x, y + 1, z)) {
                construct_face(BlockFaces::TOP, tex_atlas_index, x, y, z);
              }
              break;
            }
            case BlockFaces::LEFT: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (x == 0) {
                auto n_chunk_pos = ChunkPos{xoffset / 16 - 1, zoffset / 16};
                if (!world_chunks.at(n_chunk_pos)
                         .is_air_voxel(CHUNK_WIDTH - 1, y, z)) {
                  continue;
                }

                construct_face(BlockFaces::LEFT, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x - 1, y, z)) {
                construct_face(BlockFaces::LEFT, tex_atlas_index, x, y, z);
              }
              break;
            }
            case BlockFaces::RIGHT: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (x == CHUNK_WIDTH - 1) {
                auto n_chunk_pos = ChunkPos{xoffset / 16 + 1, zoffset / 16};
                if (!world_chunks.at(n_chunk_pos).is_air_voxel(0, y, z)) {
                  continue;
                }

                construct_face(BlockFaces::RIGHT, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x + 1, y, z)) {
                construct_face(BlockFaces::RIGHT, tex_atlas_index, x, y, z);
              }
              break;
            }
            case BlockFaces::FRONT: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (z == 0) {
                auto n_chunk_pos = ChunkPos{xoffset / 16, zoffset / 16 + 1};
                if (!world_chunks.at(n_chunk_pos)
                         .is_air_voxel(x, y, CHUNK_DEPTH - 1)) {
                  continue;
                }
                construct_face(BlockFaces::FRONT, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x, y, z - 1)) {
                construct_face(BlockFaces::FRONT, tex_atlas_index, x, y, z);
              }
              break;
            }
            case BlockFaces::BACK: {
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (z == CHUNK_DEPTH - 1) {
                auto n_chunk_pos = ChunkPos{xoffset / 16, zoffset / 16 - 1};
                if (!world_chunks.at(n_chunk_pos).is_air_voxel(x, y, 0)) {
                  continue;
                }
                construct_face(BlockFaces::BACK, tex_atlas_index, x, y, z);
                continue;
              }
              if (is_air_voxel(x, y, z + 1)) {
                construct_face(BlockFaces::BACK, tex_atlas_index, x, y, z);
              }
              break;
            }
          }
        }
      }
    }
  }
}
