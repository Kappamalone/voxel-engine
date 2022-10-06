#include "chunk_manager.h"
#include "chunk.h"
#include <thread>

ChunkManager::ChunkManager(PlayerCamera& player_camera)
    : player_camera(player_camera),
      shader_program(chunk_vert, chunk_frag, ShaderSourceType::STRING),
      perlin_noise(random_seed()) {

  mesh_gen_thread = std::thread(
      [](std::deque<Chunk*>& mesh_gen_queue) {
        while (true) {
          while (!mesh_gen_queue.empty()) {
            mesh_gen_queue[0]->create_mesh();
            mesh_gen_queue.pop_front();
          }
          PRINT("");
        }
      },
      std::ref(mesh_gen_queue));
  mesh_gen_thread.detach();

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
  unsigned char* pixels =
      stbi_load("../src/assets/terrain.png", &width, &height, &channels, 0);
  if (!pixels) {
    PANIC("Cannot find texture!\n");
  }
  PRINT("[DEBUG] Texture atlas (width, height, channels): ({}, {}, {})\n",
        width, height, channels);
  glTextureStorage2D(tex_atlas, 1, GL_RGBA8, width, height);
  glTextureSubImage2D(tex_atlas, 0, 0, 0, width, height, GL_RGBA,
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
  visible_list.clear();
  render_list.clear();
  structures_to_be_generated.clear();
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

  // NOTE: we create voxel data for radius view_distance+1, but only generate
  // voxel information for view_distance in order to cull chunk borders

  //  TODO: this is the next major performance bottleneck
  // voxel creation pass
  double before = glfwGetTime();
  for (int dx = -view_distance - 1; dx <= view_distance + 1; ++dx) {
    for (int dz = -view_distance - 1; dz <= view_distance + 1; ++dz) {
      auto w =
          ChunkPos{.x = world_chunk_pos.x + dx, .z = world_chunk_pos.z + dz};

      if (world_chunks.find(w) == world_chunks.end()) {
        auto chunk = Chunk(w, perlin_noise);
        world_chunks.insert({w, chunk});
      }
    }
  }

  /*
  // TODO: add to vector for each frame
  for (const auto& structure : structures_to_be_generated) {
    switch (structure.structure_type) {
      case StructureType::TREE:
        break;
    }
  }
  */

  double after = glfwGetTime();
  if ((after - before) * 1000 > 5) {
    PRINT("Voxel Creation: {}\n", (after - before) * 1000);
  }

  // visible chunks pass and mesh creation pass
  before = glfwGetTime();
  for (int dx = -view_distance; dx <= view_distance; ++dx) {
    for (int dz = -view_distance; dz <= view_distance; ++dz) {
      auto w =
          ChunkPos{.x = world_chunk_pos.x + dx, .z = world_chunk_pos.z + dz};

      auto& chunk = world_chunks.at(w);
      if (!chunk.initial_mesh_created() && !chunk.has_mesh_requested()) {
        for (const auto& structure : chunk.get_structures()) {
          place_structure_within_chunk(w, structure.x, structure.y, structure.z,
                                       structure.structure_type);
        }

        auto& f_chunk = world_chunks.at(ChunkPos{.x = w.x, .z = w.z + 1});
        auto& b_chunk = world_chunks.at(ChunkPos{.x = w.x, .z = w.z - 1});
        auto& l_chunk = world_chunks.at(ChunkPos{.x = w.x - 1, .z = w.z});
        auto& r_chunk = world_chunks.at(ChunkPos{.x = w.x + 1, .z = w.z});
        chunk.set_neighbour_chunks(&f_chunk, &b_chunk, &l_chunk, &r_chunk);
        chunk.request_mesh_creation();
        mesh_gen_queue.push_back(&chunk);
      } else {
        visible_list.push_back(ChunkDrawData{.chunk = &chunk});
      }
    }
  }
  after = glfwGetTime();
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
  if ((after - before) * 1000 > 5) {
    PRINT("Voxel Mesh: {}\n", (after - before) * 1000);
  }

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
