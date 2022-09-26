#pragma once
#include "common.h"
#include <vector>

struct Point {
  float x;
  float y;
  Point(float x, float y) {
    if (x > 1.0f || x < -1.0f) {
      PANIC("Lerp point not between -1 and 1!\n");
    }
    this->x = x;
    this->y = y;
  }
};

// a poor man's spline interpolator
class LerpPoints {
  std::vector<Point> points;

public:
  LerpPoints(Point first, Point last) {
    if (first.x != -1.0f || last.x != 1.0f) {
      PANIC("invalid starting arguments for lerp points!\n");
    }

    points.push_back(first);
    points.push_back(last);
  }
  void add_point(Point point) {
    if (points.empty()) {
      points.push_back(point);
    } else {
      // perform linear search to find insertion point
      int index;
      for (auto i = 0; i < points.size(); i++) {
        if (point.x >= points[i].x) {
          index = i;
        }
      }
      points.insert(points.begin() + index + 1, 1, point);
    }
  }

  float interpolate(float x) {
    if (x > 1.0f || x < -1.0f) {
      PANIC("Interpolation point not between -1 and 1!\n");
    }

    float x0, y0, x1, y1;
    Point lower = Point(-1, -1);
    Point upper = Point(-1, -1);
    for (auto i = 0; i < points.size() - 1; i++) {
      if (points[i].x <= x && x <= points[i + 1].x) {
        lower = points[i];
        upper = points[i + 1];
      }
    }

    float gradient = (upper.y - lower.y) / (upper.x - lower.x);
    float value = lower.y + (x - lower.x) * gradient;
    return value;
  }

  void print_points() {
    for (auto& point : points) {
      PRINT("{} ", point.x);
    }
    PRINT("\n");
  }
};
