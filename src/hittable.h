#ifndef HITTABLE_H
#define HITTABLE_H

#include "utility.h"

class hit_record {
  public:
    point3 p;
    vec3 normal;
    double t;
    bool front_face;
    void set_face_normal(const ray& r, const vec3& outward_normal){
        if (dot(r.direction(), outward_normal) > 0.0) {
            // ray is inside the sphere
            normal = -outward_normal;
            front_face = false;
        } else {
            // ray is outside the sphere
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

#endif