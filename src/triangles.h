#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "hittable.h"

class triangle : public hittable {
  public:
    // v0,v1,v2 - vertex positions
    // n0,n1,n2 - per-vertex normals
    // uv0,uv1,uv2 - texture coordinates
    triangle(const point3& v0, const point3& v1, const point3& v2,
             const vec3& n0, const vec3& n1, const vec3& n2,
             double u0, double v0t, double u1, double v1t,
             double u2, double v2t,
             const Material& mat)
        : v0(v0), v1(v1), v2(v2),
          n0(n0), n1(n1), n2(n2),
          u0(u0), v0t(v0t), u1(u1), v1t(v1t), u2(u2), v2t(v2t),
          mat(mat)
    {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Möller–Trumbore algorithm
        const double EPSILON = 1e-8;
        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;

        vec3 h = cross(r.direction(), edge2);
        double a = dot(edge1, h);

        if (std::fabs(a) < EPSILON) return false;

        double f = 1.0 / a;
        vec3 s = r.origin() - v0;
        double u = f * dot(s, h);

        if (u < 0.0 || u > 1.0) return false;

        vec3 q = cross(s, edge1);
        double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0) return false;

        double t = f * dot(edge2, q);
        if (!ray_t.surrounds(t)) return false;

        double w = 1.0 - u - v;

        rec.t = t;
        rec.p = r.at(t);
        rec.mat = &mat;

        vec3 interp_normal = unit_vector(w * n0 + u * n1 + v * n2);
        rec.set_face_normal(r, interp_normal);

        rec.u = w * u0 + u * u1 + v * u2;
        rec.v = w * v0t + u * v1t + v * v2t;

        return true;
    }

  private:
    point3 v0, v1, v2;
    vec3 n0, n1, n2;
    double u0, v0t, u1, v1t, u2, v2t;
    Material mat;
};

#endif