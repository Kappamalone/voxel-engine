#include "chunk.h"
#include "common.h"
#include <algorithm>
#include <cmath>

Chunk::Chunk(glm::mat4 model, glm::mat4 projection)
    : model(model), projection(projection),
      shader_program(chunk_vert, chunk_frag, ShaderSourceType::STRING) {
  // texture atlas
  // TODO: abstract this away into a class?
  glCreateTextures(GL_TEXTURE_2D, 1, &tex_atlas);
  glTextureParameteri(tex_atlas, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(tex_atlas, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(tex_atlas, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(tex_atlas, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // texture atlas must be a power of 2, and have equal width and height eg:
  // 256x256
  int width, height, channels;
  // stbi_set_flip_vertically_on_load(true);
  unsigned char* pixels =
      stbi_load("../src/assets/terrain.png", &width, &height, &channels, 0);
  if (!pixels) {
    PANIC("Cannot find texture!\n");
  }
  PRINT("[DEBUG] Texture atlas (width, height, channels): ({}, {}, {})\n",
        width, height, channels);
  tex_atlas_rows = std::sqrt(width);
  glTextureStorage2D(tex_atlas, 1, GL_RGBA8, width, height);
  glTextureSubImage2D(tex_atlas, 0, 0, 0, width, height, GL_RGBA,
                      GL_UNSIGNED_BYTE, pixels);
  stbi_image_free(pixels);
  // TODO: mipmap?

  create_voxels();
  create_mesh();

  // FIXME: GL_STATIC_DRAW?
  glCreateVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);
  glNamedBufferData(vbo, vertices_buffer.size() * 4, vertices_buffer.data(),
                    GL_STATIC_DRAW);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0,
                            sizeof(float) * attributes_per_vertice);

  // vertice
  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  // tex coord
  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
  glVertexArrayAttribBinding(vao, 1, 0);

  shader_program.set_uniform_matrix<UniformMSize::FOUR>("model", 1, false,
                                                        glm::value_ptr(model));
  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "projection", 1, false, glm::value_ptr(projection));
}

void Chunk::render_mesh(glm::mat4& view) {
  shader_program.set_uniform_matrix<UniformMSize::FOUR>("view", 1, false,
                                                        glm::value_ptr(view));

  shader_program.use();
  glBindVertexArray(vao);
  glBindTextureUnit(0, tex_atlas);
  glDrawArrays(GL_TRIANGLES, 0,
               vertices_buffer.size() / attributes_per_vertice);
}

void Chunk::create_voxels() {
  voxels.resize(CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT);
  for (auto y = 0; y < CHUNK_HEIGHT; y++) {
    for (auto z = 0; z < CHUNK_DEPTH; z++) {
      for (auto x = 0; x < CHUNK_WIDTH; x++) {
        voxels[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_DEPTH] =
            Voxel{.voxel_type = VoxelType::GRASS};
      }
    }
  }
}

void Chunk::construct_face(BlockFaces face, int atlas_index, float x, float y,
                           float z) {
  using fp =
      void(std::vector<float> & vertices_buffer, float x, float y, float z);
  static fp* vertices[] = {
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH);
      },
      // repeat
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back((-z) * VOXEL_LENGTH - VOXEL_LENGTH);
      },
      [](std::vector<float>& vertices_buffer, float x, float y, float z) {
        vertices_buffer.push_back(x * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(y * VOXEL_LENGTH + VOXEL_LENGTH);
        vertices_buffer.push_back(-z * VOXEL_LENGTH);
      },
  };

  switch ((BlockFaces)face) {
    case BlockFaces::BOTTOM:
      vertices[2](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[3](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[0](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[0](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[1](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[2](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::TOP:
      vertices[7](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[6](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[5](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[5](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[4](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[7](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::LEFT:
      vertices[0](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[4](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[5](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[5](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[1](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[0](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::RIGHT:
      vertices[2](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[6](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[7](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[7](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[3](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[2](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::FRONT:
      vertices[3](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[7](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[4](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[4](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[0](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[3](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
    case BlockFaces::BACK:
      vertices[1](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      vertices[5](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_RIGHT, atlas_index);
      vertices[6](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);

      vertices[6](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::TOP_LEFT, atlas_index);
      vertices[2](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_LEFT, atlas_index);
      vertices[1](vertices_buffer, x, y, z);
      emit_texture_coordinates(TexturePosition::BOTTOM_RIGHT, atlas_index);
      break;
  }
}

void Chunk::emit_texture_coordinates(TexturePosition position,
                                     int atlas_index) {
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

void Chunk::create_mesh() {

  // algorithm:
  //  for each voxel that isn't an air type, check if any of it's six faces
  //  borders an air block, if so add that face to the mesh, else ignore
  //
  //  TODO: texturing and texture atlases
  //  TODO: handle chunk border vertices culling

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
