#pragma once
#include <string>
#include <vector>

struct Vec3  { float x = 0, y = 0, z = 0; };
struct Color { float r = 0, g = 0, b = 0; };

enum class TransformType { Translate, Scale, RotateX, RotateY, RotateZ };
struct Transform {
    TransformType type;
    float x=0, y=0, z=0, theta=0;
};

struct Camera {
    Vec3 position, lookat, up;
    float hfov = 45.0f;
    int resH = 800, resV = 600;
    int maxBounces = 5;
    std::vector<Transform> transforms;
};

struct PhongParams { float ka=0.2f, kd=0.8f, ks=0.5f, exponent=32.0f; };

struct SceneMaterial {
    bool textured = false;
    std::string textureName;
    Color color;
    PhongParams phong;
    float reflectance = 0.0f;
    float transmittance = 0.0f;
    float ior = 1.0f;
};

struct Sphere {
    float radius = 1.0f;
    Vec3 position;
    SceneMaterial material;
    std::vector<Transform> transforms;
};

struct Mesh {
    std::string name;
    SceneMaterial material;
    std::vector<Transform> transforms;
};

struct AmbientLight { Color color; };
struct ParallelLight { Color color; Vec3 direction; };
struct PointLight { Color color; Vec3 position; };
struct SpotLight {
    Color color; Vec3 position, direction;
    float alpha1=0, alpha2=0;
};

struct Scene {
    std::string outputFile = "output.ppm";
    Color background;
    Camera camera;
    bool hasAmbient = false;
    AmbientLight ambientLight;
    std::vector<ParallelLight> parallelLights;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    std::vector<Sphere> spheres;
    std::vector<Mesh> meshes;
};
