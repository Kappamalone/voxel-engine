#include "chunk.h"
#include "common.h"
#include "lerp_points.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

Chunk::Chunk(ChunkPos chunk_pos, siv::PerlinNoise& perlin_noise)
    : chunk_pos(chunk_pos), perlin_noise(perlin_noise) {
  create_voxels();
  bounding_box = BoundingBox{
      .min = glm::vec3(get_x_offset(), 0, get_z_offset()),
      .max = glm::vec3(get_x_offset(), CHUNK_HEIGHT, get_z_offset())};
}

void Chunk::set_neighbour_chunks(Chunk* u_chunk, Chunk* d_chunk, Chunk* l_chunk,
                                 Chunk* r_chunk) {
  this->f_chunk = u_chunk;
  this->b_chunk = d_chunk;
  this->l_chunk = l_chunk;
  this->r_chunk = r_chunk;
}

// TODO: trees
// mark locations for trees to be placed by chunk manager
// (mark local x y, and world y coord)
void Chunk::create_voxels() {
  voxels.resize(CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT);
  std::uniform_real_distribution<double> unif(0, 1);
  std::random_device rand_dev;
  std::mt19937 rand_engine(rand_dev());

  LerpPoints lerp_points(Point(-1.0f, 60), Point(1.0f, 120));
  lerp_points.add_point(Point(0.0, 90));
  lerp_points.add_point(Point(0.2, 95));
  lerp_points.add_point(Point(0.4, 90));

  for (auto z = 0; z < CHUNK_DEPTH; z++) {
    for (auto x = 0; x < CHUNK_WIDTH; x++) {
      static constexpr int WATER_THRESHOLD = 90;
      static constexpr float PERLIN_SCALE = 0.035f;
      static constexpr float PERLIN_OCTAVES = 3;
      static constexpr float PERLIN_PERSISTENCE = 0.5;

      double perlin_value =
          (perlin_noise.octave2D_11((get_x_offset() + x) * PERLIN_SCALE,
                                    (get_z_offset() - z) * PERLIN_SCALE,
                                    PERLIN_OCTAVES, PERLIN_PERSISTENCE));
      int height = lerp_points.interpolate(perlin_value);

      for (auto y = 0; y < std::max(height, WATER_THRESHOLD); y++) {
        /*
        // TODO: figure out how squishification works
        static constexpr double DENSITY_THRESHOLD = 0.35f;
        double density = perlin_noise.octave3D_11(
            (get_x_offset() + x) * PERLIN_SCALE, y * PERLIN_SCALE,
            (get_z_offset() - z) * PERLIN_SCALE, PERLIN_OCTAVES,
        PERLIN_PERSISTENCE); if (density >= DENSITY_THRESHOLD) {
          set_voxel(x, y, z, VoxelType::GRASS);
        }
        */
        if (y < height) {
          if (y <= WATER_THRESHOLD) {
            set_voxel(x, y, z, VoxelType::DIRT);
          } else if (y >= height - 1) {
            set_voxel(x, y, z, VoxelType::GRASS);
            if (unif(rand_engine) > 0.99) {
              structures.emplace_back(x, y + 1, z, StructureType::TREE);
            }
          } else if (y >= height - 5) {
            set_voxel(x, y, z, VoxelType::DIRT);
          } else {
            set_voxel(x, y, z, VoxelType::STONE);
          }
        } else {
          set_voxel(x, y, z, VoxelType::WATER);
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
      vertices_buffer.push_back(get_x_offset() + x);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(get_z_offset() - z);
      break;
    case 1:
      vertices_buffer.push_back(get_x_offset() + x);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(get_z_offset() - z - 1.0f);
      break;
    case 2:
      vertices_buffer.push_back(get_x_offset() + x + 1.0f);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(get_z_offset() - z - 1.0f);
      break;
    case 3:
      vertices_buffer.push_back(get_x_offset() + x + 1.0f);
      vertices_buffer.push_back(y);
      vertices_buffer.push_back(get_z_offset() - z);
      break;
    case 4:
      vertices_buffer.push_back(get_x_offset() + x);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(get_z_offset() - z);
      break;
    case 5:
      vertices_buffer.push_back(get_x_offset() + x);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(get_z_offset() - z - 1.0f);
      break;
    case 6:
      vertices_buffer.push_back(get_x_offset() + x + 1.0f);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(get_z_offset() - z - 1.0f);
      break;
    case 7:
      vertices_buffer.push_back(get_x_offset() + x + 1.0f);
      vertices_buffer.push_back(y + 1.0f);
      vertices_buffer.push_back(get_z_offset() - z);
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
    // PRINT("Threading is hard...\n");
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
                if (!(l_chunk->is_air_voxel(CHUNK_WIDTH - 1, y, z))) {
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
                if (!(r_chunk->is_air_voxel(0, y, z))) {
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
              // NOTE: front faces the player, back faces away (d'oh)
              int tex_atlas_index = tex_atlas_map[(BlockFaces)face];
              if (z == 0) {
                if (!(f_chunk->is_air_voxel(x, y, CHUNK_DEPTH - 1))) {
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
                if (!(b_chunk->is_air_voxel(x, y, 0))) {
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
