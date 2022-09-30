#include "chunk_manager.h"
#include "chunk.h"
#include <random>
#include <thread>

ChunkManager::ChunkManager(PlayerCamera& player_camera)
    : player_camera(player_camera),
      shader_program(chunk_vert, chunk_frag, ShaderSourceType::STRING),
      perlin_noise(random_seed()) {

  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "projection", 1, false,
      glm::value_ptr(*player_camera.get_projection_matrix()));

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
  unsigned char* pixels = stbi_load("../src/assets/terrain_2071786.jpg", &width,
                                    &height, &channels, 0);
  if (!pixels) {
    PANIC("Cannot find texture!\n");
  }
  PRINT("[DEBUG] Texture atlas (width, height, channels): ({}, {}, {})\n",
        width, height, channels);
  glTextureStorage2D(tex_atlas, 1, GL_RGB8, width, height);
  glTextureSubImage2D(tex_atlas, 0, 0, 0, width, height, GL_RGB,
                      GL_UNSIGNED_BYTE, pixels);
  glGenerateTextureMipmap(tex_atlas);
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
  cpu_bytes_allocated = 0;
  ChunkPos world_chunk_pos;

  if (pos.x >= 0) {
    world_chunk_pos.x = (int)(pos.x / CHUNK_WIDTH);
  } else {
    world_chunk_pos.x = floor(pos.x / CHUNK_WIDTH);
  }

  if (pos.z >= 0) {
    world_chunk_pos.z = ceil(pos.z / CHUNK_DEPTH);
  } else {
    world_chunk_pos.z = (int)(pos.z / CHUNK_DEPTH);
  }

  /*
  if (world_chunk_pos == old_world_pos) {
    return;
  }
  */

  // NOTE: there are infinitely better ways to handle chunks than to
  // rerender each of them each frame
  visible_list.clear();
  render_list.clear();
  debug_info.clear();
  old_world_pos = world_chunk_pos;

  // NOTE: we create voxel data for radius view_distance+1, but only generate
  // voxel information for view_distance in order to cull chunk borders

  // voxel creation pass
  double before = glfwGetTime();
  for (int dx = -view_distance - 1; dx <= view_distance + 1; ++dx) {
    for (int dz = -view_distance - 1; dz <= view_distance + 1; ++dz) {
      auto w =
          ChunkPos{.x = world_chunk_pos.x + dx, .z = world_chunk_pos.z + dz};

      if (world_chunks.find(w) == world_chunks.end()) {
        world_chunks.insert({w, Chunk(world_chunks, w.x * CHUNK_WIDTH,
                                      w.z * CHUNK_DEPTH, perlin_noise)});
      }
    }
  }
  double after = glfwGetTime();
  debug_info.emplace_back("Voxel Creation: ", (after - before) * 1000);
  if ((after - before) * 1000 > 5) {
    PRINT("Voxel Creation: {}\n", (after - before) * 1000);
  }

  // visible chunks pass and mesh creation pass
  before = glfwGetTime();
  static std::vector<std::thread> thread_pool;
  for (int dx = -view_distance; dx <= view_distance; ++dx) {
    for (int dz = -view_distance; dz <= view_distance; ++dz) {
      auto w =
          ChunkPos{.x = world_chunk_pos.x + dx, .z = world_chunk_pos.z + dz};

      auto& chunk = world_chunks.at(w);
      if (!chunk.initial_mesh_created()) {
        auto func = [&]() { chunk.create_mesh(); };
        thread_pool.emplace_back(func);
      }

      visible_list.push_back(ChunkDrawData{.chunk = &world_chunks.at(w)});
    }
  }
  for (auto& thread : thread_pool) {
    thread.join();
  }
  thread_pool.clear();
  after = glfwGetTime();

  debug_info.emplace_back("Voxel Mesh: ", (after - before) * 1000);
  if ((after - before) * 1000 > 5) {
    PRINT("Voxel Mesh: {}\n", (after - before) * 1000);
  }

  before = glfwGetTime();
  player_camera.update_frustum();
  for (auto& i : visible_list) {
    if (player_camera.frustum.test_bounding_box(i.chunk->get_bounding_box())) {
      render_list.push_back(i);
    }
  }
  after = glfwGetTime();
  debug_info.emplace_back("Voxel BB: ", (after - before) * 1000);

  for (auto& drawable : render_list) {
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

void ChunkManager::render_chunks() {
  manage_chunks(player_camera.get_player_pos());

  shader_program.set_uniform_matrix<UniformMSize::FOUR>(
      "view", 1, false, glm::value_ptr(*player_camera.get_view_matrix()));

  shader_program.use();
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindTextureUnit(0, tex_atlas);

  std::vector<GLsizei> first;
  std::vector<GLsizei> count;
  for (auto& drawable : render_list) {
    first.push_back(drawable.offset);
    count.push_back((drawable.chunk->get_vertices_byte_size() / sizeof(float)) /
                    attributes_per_vertice);
  }
  glMultiDrawArrays(GL_TRIANGLES, first.data(), count.data(), first.size());
}

uint32_t ChunkManager::random_seed() {
  std::uniform_real_distribution<double> unif(0, 1);
  std::random_device rand_dev;
  std::mt19937 rand_engine(rand_dev());
  uint32_t x = unif(rand_engine) * 0xffff'ffff;
  PRINT("[DEBUG] seed: {}\n", x);
  return x;
}
