#ifndef CAMERA_H
#define CAMERA_H

#include <unordered_map>

#include "mat4.h"
#include "texture.h"
#include "hittable.h"
#include "core/Scene.h" 

inline vec3  toVec3 (const Vec3&  v) { return vec3 (v.x, v.y, v.z); }
inline color toColor(const Color& c) { return color(c.r, c.g, c.b); }
inline vec3 refract_dir(const vec3& d, const vec3& n, double eta) {
    double cos_theta = std::fmin(dot(-d, n), 1.0);
    vec3 r_perp = eta * (d + cos_theta * n);
    vec3 r_parallel = -std::sqrt(std::fabs(1.0 - r_perp.length_squared())) * n;
    return r_perp + r_parallel;
}

class camera {
  public:
    point3 position = {0,0,0};
    point3 lookat = {0,0,-1};
    vec3 up = {0,1,0};
    double hfov = 45.0;
    std::vector<Transform> transforms;
    // Image
    int image_width = 400;
    int image_height;
    int samples_per_pixel = 10;
    int max_bounces = 5;
    std::string output_file = "output.ppm";
    color background = color(0,0,0);

    bool has_ambient = false;
    color ambient_color = color(0,0,0);
    std::vector<PointLight> point_lights;
    std::vector<ParallelLight> parallel_lights;
    std::vector<SpotLight> spot_lights;

    std::unordered_map<std::string, std::shared_ptr<Texture>> texture_cache;

    void render(const hittable& world) {
        initialize();
        std::ofstream out(output_file);

        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++){
                write_color(out, ray_color(get_ray(i, j), world, max_bounces));
            }
        }
        std::clog << "\rDone.                              \n";
    }

  private:
    double pixel_samples_scale;
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;

    void initialize() {
        if (!transforms.empty()) {
            Mat4 M = mat4_from_transforms(transforms);
            position = M.apply_point(position);
            lookat = M.apply_point(lookat);
            up = M.apply_vector(up);
        }

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = point3(0, 0, 0);

        vec3 w = unit_vector(position - lookat);
        vec3 u = unit_vector(cross(up, w));
        vec3 v = cross(w, u);

        double half_w = std::tan(degrees_to_radians(hfov));
        double half_h = half_w * (double(image_height) / image_width);

        vec3 viewport_h = 2.0 * half_w * u;
        vec3 viewport_v2 = 2.0 * half_h * v;

        pixel_delta_u = viewport_h / image_width;
        pixel_delta_v = -(viewport_v2 / image_height);  // flip: iterate top-down

        point3 vp_upper_left = position - w - viewport_h / 2.0 + viewport_v2 / 2.0;
        pixel00_loc = vp_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    ray get_ray(int i, int j){
        point3 pc = pixel00_loc + double(i)*pixel_delta_u + double(j)*pixel_delta_v;
        return ray(position, pc - position);
    }

    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    color ray_color(const ray& r, const hittable& world, int bounces) const {
        if (bounces <= 0) return color(0,0,0);

        hit_record rec;
        if (!world.hit(r, interval(1e-4, infinity), rec))
            return background;

        const Material& mat = *rec.mat;
        vec3 N = unit_vector(rec.normal);
        vec3 V = unit_vector(-r.direction());
        color result(0,0,0);

        // 1. Ambient
        color surface_color = mat.albedo;
        if (mat.textured && !mat.textureName.empty()) {
            auto it = texture_cache.find(mat.textureName);
            if (it != texture_cache.end()){
                surface_color = it->second->sample(rec.u, rec.v);
            }
        }

        if (has_ambient)
            result = mat.ka * ambient_color * surface_color;

        // 2. Point lights
        for (const auto& light : point_lights) {
            vec3 Lv = toVec3(light.position) - rec.p;
            double dist = Lv.length();
            vec3 L = unit_vector(Lv);
            if (in_shadow(rec.p, L, dist, world)) continue;

            color lc = toColor(light.color);
            double diff = std::fmax(0.0, dot(N, L));
            result += mat.kd * diff * (surface_color * lc);

            vec3 H = unit_vector(L + V);
            double spec = std::pow(std::fmax(0.0, dot(N, H)), mat.exponent);
            result += mat.ks * spec * lc;
        }

        // 3. Parallel lights
        for (const auto& light : parallel_lights) {
            vec3 L = unit_vector(-toVec3(light.direction));
            if (in_shadow(rec.p, L, infinity, world)) continue;

            color lc = toColor(light.color);
            double diff = std::fmax(0.0, dot(N, L));
            result += mat.kd * diff * (surface_color * lc);

            vec3 H = unit_vector(L + V);
            double spec = std::pow(std::fmax(0.0, dot(N, H)), mat.exponent);
            result += mat.ks * spec * lc;
        }

        // 4. Spot lights
        for (const auto& light : spot_lights) {
            vec3 Lv   = toVec3(light.position) - rec.p;
            double dist = Lv.length();
            vec3 L    = unit_vector(Lv);
            if (in_shadow(rec.p, L, dist, world)) continue;

            // Angle between spot direction and the vector TO the hit point
            vec3 spot_dir = unit_vector(toVec3(light.direction));
            double cos_angle = dot(spot_dir, -L);   // -L = direction from light to surface
            double angle_deg = degrees_to_radians(0); // convert back for comparison
            double alpha1_cos = std::cos(degrees_to_radians(light.alpha1));
            double alpha2_cos = std::cos(degrees_to_radians(light.alpha2));

            // Outside outer cone: no contribution
            if (cos_angle < alpha2_cos) continue;

            // Smooth falloff between inner (alpha1) and outer (alpha2) cone
            double intensity = 1.0;
            if (cos_angle < alpha1_cos) {
                double t = (cos_angle - alpha2_cos) / (alpha1_cos - alpha2_cos);
                intensity = t * t;  // quadratic falloff
            }

            color lc = toColor(light.color);
            double diff = std::fmax(0.0, dot(N, L));
            result += intensity * mat.kd * diff * (surface_color * lc);

            vec3 H = unit_vector(L + V);
            double spec = std::pow(std::fmax(0.0, dot(N, H)), mat.exponent);
            result += intensity * mat.ks * spec * lc;
        }

        // reflection and refraction 

        if (mat.reflectance > 0.0) {
            vec3 reflected = r.direction() - 2.0 * dot(r.direction(), N) * N;
            ray  reflect_ray(rec.p, reflected);
            color reflect_color = ray_color(reflect_ray, world, bounces - 1);
            result = (1.0 - mat.reflectance) * result + mat.reflectance * reflect_color;
        }

        if (mat.transmittance > 0.0) {
            vec3 ray_dir = unit_vector(r.direction());
            
            vec3 N_refract;
            double eta;
            if (rec.front_face) {
                N_refract = N;           // entering: normal points outward, against ray
                eta = 1.0 / mat.ior;     // air -> material
            } else {
                N_refract = N;          // exiting: flip normal to point against ray
                eta = mat.ior / 1.0;     // material -> air
            }

            double cos_theta = std::fmin(dot(-ray_dir, N_refract), 1.0);
            double sin2_theta_t = eta * eta * (1.0 - cos_theta * cos_theta);

            color refract_color;
            if (sin2_theta_t > 1.0) {
                vec3 reflected = ray_dir - 2.0 * dot(ray_dir, N_refract) * N_refract;
                refract_color = ray_color(ray(rec.p + 1e-4 * reflected, reflected), world, bounces - 1);
            } else {
                vec3 refracted = refract_dir(ray_dir, N_refract, eta);
                refract_color = ray_color(ray(rec.p + 1e-4 * refracted, refracted), world, bounces - 1);
            }

            result = (1.0 - mat.transmittance) * result + mat.transmittance * refract_color;
        }

        return result;
    }

    bool in_shadow(const point3& origin, const vec3& dir, double max_dist, const hittable& world) const {
        hit_record tmp;
        return world.hit(ray(origin, dir), interval(1e-4, max_dist - 1e-4), tmp);
    }

};

#endif 