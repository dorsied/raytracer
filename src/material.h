#ifndef MATERIAL_H
#define MATERIAL_H

#include "utility.h"

struct Material {
    color albedo = color(1, 1, 1);

    // Phong coefficients
    double ka = 0.2;   // ambient
    double kd = 0.8;   // diffuse
    double ks = 0.5;   // specular
    double exponent = 32.0;  // shininess

    double reflectance = 0.0;
    double transmittance = 0.0;
    double ior = 1.0;

    bool textured = false;
    std::string textureName;
};

#endif
