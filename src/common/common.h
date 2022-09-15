#pragma once
#include "fmt/core.h"

/*
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
*/

#ifndef NDEBUG
#define DPRINT(...) fmt::print(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#define PRINT(...) fmt::print(__VA_ARGS__)

// clang-format off
#define PANIC(...) do {     \
  fmt::print(__VA_ARGS__);  \
  exit(1);                  \
} while (0)                 \
//clang-format on
