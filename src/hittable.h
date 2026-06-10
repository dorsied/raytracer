#ifndef HITTABLE_H
#define HITTABLE_H

#include "mat4.h"
#include "utility.h"
#include "camera.h"
#include "material.h"

class hit_record {
  public:
    point3 p;
    vec3 normal;
    double t;
    bool front_face;

    const Material* mat = nullptr;

    double u = 0, v = 0;

    void set_face_normal(const ray& r, const vec3& outward_normal){
        if (dot(r.direction(), outward_normal) > 0.0) {
            // inside the sphere
            normal = -outward_normal;
            front_face = false;
        } else {
            // outside the sphere
            normal = outward_normal;
            front_face = true;
        }
    }
};

class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

class transformed : public hittable {
public:
    transformed(std::shared_ptr<hittable> obj, const Mat4& M) : object(obj), M(M), M_inv(M.inverse()), M_inv_T(M.inverse().transpose()) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        ray local_r = M_inv.apply_ray(r);

        if (!object->hit(local_r, ray_t, rec)){
            return false;
        }

        rec.p = M.apply_point(rec.p);
        vec3 world_normal = M_inv_T.apply_vector(rec.normal);

        rec.set_face_normal(r, unit_vector(world_normal));
        return true;
    }

private:
    std::shared_ptr<hittable> object;
    Mat4 M, M_inv, M_inv_T;
};

#endif