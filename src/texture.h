#ifndef TEXTURE_H
#define TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "utility.h"
#include <string>
#include <stdexcept>

class Texture {
  public:
    Texture() = default;

    void load(const std::string& path) {
        int channels;
        data = stbi_load(path.c_str(), &width, &height, &channels, 3);
        if (!data){
            throw std::runtime_error("Cannot load texture: " + path);
        }
    }

    ~Texture() { if (data) stbi_image_free(data); }

    color sample(double u, double v) const {
      if (!data) return color(1, 0, 1);

      u = u - std::floor(u);
      v = v - std::floor(v);

      // pixel coordinates with fractional part
      double xf = u * width - 0.5;
      double yf = v * height - 0.5;

      // top-left corner
      int x0 = (int)std::floor(xf);
      int y0 = (int)std::floor(yf);

      // fractional parts
      double tx = xf - x0;
      double ty = yf - y0;

      // clamp all four neighbours
      auto px = [&](int x, int y) -> color {
          x = ((x % width) + width) % width;
          y = ((y % height) + height) % height;
          int idx = (y * width + x) * 3;
          return color(data[idx] / 255.0,
                       data[idx + 1] / 255.0,
                       data[idx + 2] / 255.0);
      };

      color c00 = px(x0, y0);
      color c10 = px(x0 + 1, y0);
      color c01 = px(x0, y0 + 1);
      color c11 = px(x0 + 1, y0 + 1);

      // bilinear blend
      color c0 = (1 - tx) * c00 + tx * c10;
      color c1 = (1 - tx) * c01 + tx * c11;
      return (1 - ty) * c0 + ty * c1;
  }

    bool loaded() const { return data != nullptr; }

  private:
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
};

#endif