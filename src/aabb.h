#ifndef AABB_H
#define AABB_H

#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include <algorithm>

class aabb {
  public:
    interval x, y, z;

    aabb() {}

    aabb(const interval& x, const interval& y, const interval& z)
        : x(x), y(y), z(z) {}

    // build from two corner points
    aabb(const point3& a, const point3& b) {
        x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
    }

    const interval& axis_interval(int n) const {
        if (n == 0) return x;
        if (n == 1) return y;
        return z;
    }

    point3 centroid() const {
        return point3(
            (x.min + x.max) * 0.5,
            (y.min + y.max) * 0.5,
            (z.min + z.max) * 0.5
        );
    }

    double surface_area() const {
        double dx = x.size();
        double dy = y.size();
        double dz = z.size();
        return 2.0 * (dx * dy + dy * dz + dz * dx);
    }

    // merge two AABBs into the tightest enclosing AABB
    static aabb merge(const aabb& a, const aabb& b) {
        return aabb(
            interval(std::fmin(a.x.min, b.x.min), std::fmax(a.x.max, b.x.max)),
            interval(std::fmin(a.y.min, b.y.min), std::fmax(a.y.max, b.y.max)),
            interval(std::fmin(a.z.min, b.z.min), std::fmax(a.z.max, b.z.max))
        );
    }

    // slab test
    bool hit(const ray& r, interval ray_t) const {
        for (int a = 0; a < 3; a++) {
            const interval& ax = axis_interval(a);
            double inv_d = 1.0 / r.direction()[a];
            double t0 = (ax.min - r.origin()[a]) * inv_d;
            double t1 = (ax.max - r.origin()[a]) * inv_d;
            if (inv_d < 0.0) std::swap(t0, t1);
            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;
            if (ray_t.max <= ray_t.min) return false;
        }
        return true;
    }
};

#endif