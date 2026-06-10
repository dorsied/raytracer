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

        int x = int(u * width);
        int y = int(v * height);

        x = std::min(x, width  - 1);
        y = std::min(y, height - 1);

        int idx = (y * width + x) * 3;
        return color(data[idx] / 255.0,
                     data[idx + 1] / 255.0,
                     data[idx + 2] / 255.0);
    }

    bool loaded() const { return data != nullptr; }

  private:
    unsigned char* data = nullptr;
    int width  = 0;
    int height = 0;
};

#endif