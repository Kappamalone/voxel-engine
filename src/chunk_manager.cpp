#include "chunk_manager.h"

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
  manage_chunks();
}

void ChunkManager::manage_chunks() {
  render_list.emplace_back(16);
  glNamedBufferSubData(vbo, 0, render_list[0].get_vertices_byte_size(),
                       render_list[0].get_vertices_data());
  cpu_bytes_allocated += render_list[0].get_vertices_byte_size();
  if (cpu_bytes_allocated >= gpu_bytes_allocated) {
    PANIC("Not enough space allocated for vertices on gpu!\n");
  }
  glVertexArrayVertexBuffer(vao, 0, vbo, 0,
                            sizeof(float) * attributes_per_vertice);
}

void ChunkManager::render_chunks(glm::vec3 pos, glm::mat4 view) {
  shader_program.set_uniform_matrix<UniformMSize::FOUR>("view", 1, false,
                                                        glm::value_ptr(view));
  auto model = glm::mat4(1.0f);
  shader_program.set_uniform_matrix<UniformMSize::FOUR>("model", 1, false,
                                                        glm::value_ptr(model));
  shader_program.use();
  glBindVertexArray(vao);
  glBindTextureUnit(0, tex_atlas);
  // TODO: glMultiDrawArrays with model matrix array uniform
  // shader_program.set_uniform_matrix<UniformMSize::FOUR>(
  //     "models", MAX_SIZE, false, glm::value_ptr(models));
  glDrawArrays(GL_TRIANGLES, 0,
               (cpu_bytes_allocated / sizeof(float)) / attributes_per_vertice);
}
