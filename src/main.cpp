#include "utility.h"

#include "mat4.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "mesh.h"
#include "parser/SceneParser.h"


int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <scene.xml>\n"; return 1; }

    Scene scene;
    try { scene = SceneParser::parse(argv[1]); }
    catch (const std::exception& e) { std::cerr << "Parse error: " << e.what() << "\n"; return 1; }

    const auto& c = scene.camera;

    hittable_list world;

    for (const auto& s : scene.spheres) {
        const auto& sm = s.material;
        Material mat;
        mat.albedo = color(sm.color.r, sm.color.g, sm.color.b);
        mat.ka = sm.phong.ka;
        mat.kd = sm.phong.kd;
        mat.ks = sm.phong.ks;
        mat.exponent = sm.phong.exponent;
        mat.reflectance = sm.reflectance;
        mat.transmittance = sm.transmittance;
        mat.ior = sm.ior;
        mat.textured = sm.textured;
        mat.textureName = sm.textureName;
        auto sph = make_shared<sphere>(
            point3(s.position.x, s.position.y, s.position.z), s.radius, mat);

        if (!s.transforms.empty()) {
            Mat4 M = mat4_from_transforms(s.transforms);
            world.add(make_shared<transformed>(sph, M));
        } else {
            world.add(sph);
        }
    }

    for (const auto& m : scene.meshes) {
        const auto& sm = m.material;
        Material mat;
        mat.albedo = color(sm.color.r, sm.color.g, sm.color.b);
        mat.ka = sm.phong.ka;
        mat.kd = sm.phong.kd;
        mat.ks = sm.phong.ks;
        mat.exponent = sm.phong.exponent;
        mat.reflectance = sm.reflectance;
        mat.transmittance = sm.transmittance;
        mat.ior = sm.ior;
        mat.textured = sm.textured;
        mat.textureName = sm.textureName;

        std::string obj_path = "scenes/" + m.name;
        try {
            hittable_list mesh_triangles = load_obj(obj_path, mat);
            Mat4 M = mat4_from_transforms(m.transforms);
            bool has_transform = !m.transforms.empty();

            for (const auto& tri : mesh_triangles.objects) {
                if (has_transform){
                    world.add(make_shared<transformed>(tri, M));
                }
                else{
                    world.add(tri);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: " << e.what() << "\n";
        }
    }

    camera cam;
    cam.position = point3(c.position.x, c.position.y, c.position.z);
    cam.lookat = point3(c.lookat.x,   c.lookat.y,   c.lookat.z);
    cam.up = vec3  (c.up.x,       c.up.y,       c.up.z);
    cam.hfov = c.hfov;
    cam.image_width = c.resH;
    cam.image_height = c.resV;
    cam.max_bounces = c.maxBounces;
    cam.output_file = scene.outputFile;
    cam.background = color(scene.background.r, scene.background.g, scene.background.b);
    cam.transforms = std::vector<Transform>(c.transforms.begin(), c.transforms.end());

    if (scene.hasAmbient) {
        cam.has_ambient = true;
        cam.ambient_color = color(scene.ambientLight.color.r, scene.ambientLight.color.g, scene.ambientLight.color.b);
    }
    cam.point_lights = scene.pointLights;
    cam.parallel_lights = scene.parallelLights;

    for (const auto& m : scene.meshes) {
        if (m.material.textured && !m.material.textureName.empty()) {
            auto tex = std::make_shared<Texture>();
            try {
                tex->load("scenes/" + m.material.textureName);
                std::cerr << "Loaded texture: " << m.material.textureName << "\n";
            } catch (const std::exception& e) {
                std::cerr << "FAILED: " << e.what() << "\n";  // ← add this
            }
            cam.texture_cache[m.material.textureName] = tex;
        }
    }


    cam.render(world);
}