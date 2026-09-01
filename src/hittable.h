#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"
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
            normal = -outward_normal;
            front_face = false;
        } else {
            normal = outward_normal;
            front_face = true;
        }
    }
};

class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
    virtual aabb bounding_box() const = 0;
};

class transformed : public hittable {
  public:
    transformed(std::shared_ptr<hittable> obj, const Mat4& M)
        : object(obj), M(M), M_inv(M.inverse()), M_inv_T(M.inverse().transpose())
    {
        // pre-compute  AABB by transforming all 8 corners
        cached_box = compute_box(obj->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        ray local_r = M_inv.apply_ray(r);
        if (!object->hit(local_r, ray_t, rec)) return false;
        rec.p = M.apply_point(rec.p);
        rec.set_face_normal(r, unit_vector(M_inv_T.apply_vector(rec.normal)));
        return true;
    }

    aabb bounding_box() const override { return cached_box; }

  private:
    std::shared_ptr<hittable> object;
    Mat4 M, M_inv, M_inv_T;
    aabb cached_box;

    aabb compute_box(const aabb& local) const {
        // transform all 8 corners, re-fit a new AABB
        double xs[2] = { local.x.min, local.x.max };
        double ys[2] = { local.y.min, local.y.max };
        double zs[2] = { local.z.min, local.z.max };

        point3 mn( infinity,  infinity,  infinity);
        point3 mx(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++) {
            point3 wc = M.apply_point(point3(xs[i], ys[j], zs[k]));
            for (int a = 0; a < 3; a++) {
                if (wc[a] < mn[a]) mn[a] = wc[a];
                if (wc[a] > mx[a]) mx[a] = wc[a];
            }
        }
        return aabb(mn, mx);
    }
};

#endif