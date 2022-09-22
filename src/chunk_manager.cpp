#include "chunk_manager.h"
#include <iostream>

ChunkManager::ChunkManager(glm::mat4 projection)
    : shader_program(chunk_vert, chunk_frag, ShaderSourceType::STRING) {
  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "projection", 1, false, glm::value_ptr(projection));

  // texture atlas
  // TODO: abstract this away into a class?
  glCreateTextures(GL_TEXTURE_2D, 1, &tex_atlas);
  glTextureParameteri(tex_atlas, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(tex_atlas, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(tex_atlas, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(tex_atlas, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // TODO: mipmap?

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

  // vertex attribute configuration
  glCreateVertexArrays(1, &vao);

  // vertice
  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(vao, 0, 0);

  // tex coord
  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
  glVertexArrayAttribBinding(vao, 1, 0);

  // gpu memory
  glCreateBuffers(1, &vbo);
  glNamedBufferData(vbo, gpu_bytes_allocated, nullptr, GL_DYNAMIC_DRAW);
  glVertexArrayVertexBuffer(vao, 0, vbo, 0,
                            sizeof(float) * attributes_per_vertice);
}

void ChunkManager::manage_chunks(glm::vec3 pos) {
  // algorithm:
  //  calculate which chunk we are in
  //  check if surrounding chunks (determined by render distance) are created
  //    if so then add it to the chunk visibility list
  //    else create them and add them to the chunk visibility list
  //  use frustum culling to determined which chunks are actually visible
  //  render chunks

  cpu_bytes_allocated = 0;
  visible_list.clear();
  render_list.clear();
  glm::vec3 world_pos;

  // TODO: please don't do this
  world_pos.y = 0.0f;
  if (pos.x >= 0) {
    world_pos.x = (int)(pos.x / CHUNK_WIDTH);
  } else {
    world_pos.x = floor(pos.x / CHUNK_WIDTH);
  }

  if (pos.z >= 0) {
    world_pos.z = ceil(pos.z / CHUNK_DEPTH);
  } else {
    world_pos.z = (int)(pos.z / CHUNK_DEPTH);
  }

  for (int dx = -view_distance; dx <= view_distance; ++dx) {
    for (int dz = -view_distance; dz <= view_distance; ++dz) {
      auto w =
          glm::vec3(world_pos.x + (float)dx, 0.0f, world_pos.z + (float)dz);

      // unordered map implicitly calls default constructor if it needs to
      visible_list.push_back(
          ChunkDrawData{.model = w, .chunk = &world_chunks[w]});
    }
  }

  // TODO: frustum culling pass

  for (auto& drawable : visible_list) {
    drawable.offset =
        cpu_bytes_allocated / sizeof(float) / attributes_per_vertice;

    auto* chunk = drawable.chunk;
    glNamedBufferSubData(vbo, cpu_bytes_allocated,
                         chunk->get_vertices_byte_size(),
                         chunk->get_vertices_data());
    cpu_bytes_allocated += chunk->get_vertices_byte_size();
    if (cpu_bytes_allocated >= gpu_bytes_allocated) {
      PANIC("Not enough space allocated for vertices on gpu!\n");
    }
  }
}

void ChunkManager::render_chunks(glm::vec3 pos, glm::mat4 view) {
  manage_chunks(pos);

  shader_program.set_uniform_matrix<UniformMSize::FOUR>("view", 1, false,
                                                        glm::value_ptr(view));

  shader_program.use();
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindTextureUnit(0, tex_atlas);

  // TODO: this data can be stored in the CHunkDrawData
  /*
  std::vector<glm::mat4> models;
  std::vector<GLsizei> first;
  std::vector<GLsizei> count;
  for (auto& drawable : visible_list) {
    models.push_back(
        glm::translate(glm::mat4(1.0f), drawable.model * (float)CHUNK_WIDTH));
    first.push_back(offset);
    offset += drawable.chunk->get_vertices_byte_size() / sizeof(float) / 5;
    count.push_back((drawable.chunk->get_vertices_byte_size() / sizeof(float)) /
                    attributes_per_vertice);
  }
  static int UNIFORM_SIZE = 64;
  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "models", UNIFORM_SIZE, false, glm::value_ptr(models[0]));
  glMultiDrawArrays(GL_TRIANGLES, first.data(), count.data(), first.size());
  */

  for (auto i = 0; i < visible_list.size(); i++) {
    auto model = glm::translate(glm::mat4(1.0f), visible_list[i].model * 16.0f);
    shader_program.set_uniform_matrix<UniformMSize::FOUR>(
        "model", 1, false, glm::value_ptr(model));
    glDrawArrays(
        GL_TRIANGLES, visible_list[i].offset,
        (visible_list[i].chunk->get_vertices_byte_size() / sizeof(float)) /
            attributes_per_vertice);
  }
  /*
  for (auto& drawable : visible_list) {
    auto model = glm::translate(glm::mat4(1.0f), drawable.model * 16.0f);
    shader_program.set_uniform_matrix<UniformMSize::FOUR>(
        "model", 1, false, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, offset,
                 (drawable.chunk->get_vertices_byte_size() / sizeof(float)) /
                     attributes_per_vertice);
    offset += drawable.chunk->get_vertices_byte_size();
  }
*/
}
