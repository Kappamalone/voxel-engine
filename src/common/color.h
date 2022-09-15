#pragma once
#include "common.h"

struct ColorRGBA {
  float r;
  float g;
  float b;
  float a;

  ColorRGBA(float r = 0., float g = 0., float b = 0., float a = 0.) {
    set(r, g, b, a);
  }

  void set(float r, float g, float b, float a) {
    for (auto i : {r, g, b, a}) {
      if (i > 1. || i < 0.) {
        PANIC("Color value outside 0-1 range: {:.2}\n", i);
      }
    }
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }

  [[nodiscard]] uint32_t to_u32() const {
    return (uint32_t)(r * 0xff) << 24 | (uint32_t)(g * 0xff) << 16 |
           (uint32_t)(b * 0xff) << 8 | (uint32_t)(a * 0xff) << 0;
  }

  // TODO: move and copy constructors
};
