#ifndef MAT4_H
#define MAT4_H

#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "utility.h"
#include "core/Scene.h"   // for Transform / TransformType

struct Mat4 {
    double m[4][4] = {};

    static Mat4 identity() {
        Mat4 r;
        r.m[0][0] = r.m[1][1] = r.m[2][2] = r.m[3][3] = 1.0;
        return r;
    }
    static Mat4 translate(double x, double y, double z) {
        Mat4 r = identity();
        r.m[0][3] = x; r.m[1][3] = y; r.m[2][3] = z;
        return r;
    }
    static Mat4 scale(double x, double y, double z) {
        Mat4 r = identity();
        r.m[0][0] = x; r.m[1][1] = y; r.m[2][2] = z;
        return r;
    }
    static Mat4 rotateX(double deg) {
        Mat4 r = identity();
        double c = std::cos(degrees_to_radians(deg)), s = std::sin(degrees_to_radians(deg));
        r.m[1][1] =  c; r.m[1][2] = -s;
        r.m[2][1] =  s; r.m[2][2] =  c;
        return r;
    }
    static Mat4 rotateY(double deg) {
        Mat4 r = identity();
        double c = std::cos(degrees_to_radians(deg)), s = std::sin(degrees_to_radians(deg));
        r.m[0][0] =  c; r.m[0][2] =  s;
        r.m[2][0] = -s; r.m[2][2] =  c;
        return r;
    }
    static Mat4 rotateZ(double deg) {
        Mat4 r = identity();
        double c = std::cos(degrees_to_radians(deg)), s = std::sin(degrees_to_radians(deg));
        r.m[0][0] =  c; r.m[0][1] = -s;
        r.m[1][0] =  s; r.m[1][1] =  c;
        return r;
    }
    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }
    Mat4 transpose() const {
        Mat4 r;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                r.m[i][j] = m[j][i];
        return r;
    }
    Mat4 inverse() const {
        double tmp[16];
        const double* s = &m[0][0];
        tmp[0]  =  s[5]*s[10]*s[15]-s[5]*s[11]*s[14]-s[9]*s[6]*s[15]+s[9]*s[7]*s[14]+s[13]*s[6]*s[11]-s[13]*s[7]*s[10];
        tmp[4]  = -s[4]*s[10]*s[15]+s[4]*s[11]*s[14]+s[8]*s[6]*s[15]-s[8]*s[7]*s[14]-s[12]*s[6]*s[11]+s[12]*s[7]*s[10];
        tmp[8]  =  s[4]*s[9]*s[15] -s[4]*s[11]*s[13]-s[8]*s[5]*s[15]+s[8]*s[7]*s[13]+s[12]*s[5]*s[11]-s[12]*s[7]*s[9];
        tmp[12] = -s[4]*s[9]*s[14] +s[4]*s[10]*s[13]+s[8]*s[5]*s[14]-s[8]*s[6]*s[13]-s[12]*s[5]*s[10]+s[12]*s[6]*s[9];
        tmp[1]  = -s[1]*s[10]*s[15]+s[1]*s[11]*s[14]+s[9]*s[2]*s[15]-s[9]*s[3]*s[14]-s[13]*s[2]*s[11]+s[13]*s[3]*s[10];
        tmp[5]  =  s[0]*s[10]*s[15]-s[0]*s[11]*s[14]-s[8]*s[2]*s[15]+s[8]*s[3]*s[14]+s[12]*s[2]*s[11]-s[12]*s[3]*s[10];
        tmp[9]  = -s[0]*s[9]*s[15] +s[0]*s[11]*s[13]+s[8]*s[1]*s[15]-s[8]*s[3]*s[13]-s[12]*s[1]*s[11]+s[12]*s[3]*s[9];
        tmp[13] =  s[0]*s[9]*s[14] -s[0]*s[10]*s[13]-s[8]*s[1]*s[14]+s[8]*s[2]*s[13]+s[12]*s[1]*s[10]-s[12]*s[2]*s[9];
        tmp[2]  =  s[1]*s[6]*s[15] -s[1]*s[7]*s[14] -s[5]*s[2]*s[15]+s[5]*s[3]*s[14]+s[13]*s[2]*s[7] -s[13]*s[3]*s[6];
        tmp[6]  = -s[0]*s[6]*s[15] +s[0]*s[7]*s[14] +s[4]*s[2]*s[15]-s[4]*s[3]*s[14]-s[12]*s[2]*s[7] +s[12]*s[3]*s[6];
        tmp[10] =  s[0]*s[5]*s[15] -s[0]*s[7]*s[13] -s[4]*s[1]*s[15]+s[4]*s[3]*s[13]+s[12]*s[1]*s[7] -s[12]*s[3]*s[5];
        tmp[14] = -s[0]*s[5]*s[14] +s[0]*s[6]*s[13] +s[4]*s[1]*s[14]-s[4]*s[2]*s[13]-s[12]*s[1]*s[6] +s[12]*s[2]*s[5];
        tmp[3]  = -s[1]*s[6]*s[11] +s[1]*s[7]*s[10] +s[5]*s[2]*s[11]-s[5]*s[3]*s[10]-s[9]*s[2]*s[7]  +s[9]*s[3]*s[6];
        tmp[7]  =  s[0]*s[6]*s[11] -s[0]*s[7]*s[10] -s[4]*s[2]*s[11]+s[4]*s[3]*s[10]+s[8]*s[2]*s[7]  -s[8]*s[3]*s[6];
        tmp[11] = -s[0]*s[5]*s[11] +s[0]*s[7]*s[9]  +s[4]*s[1]*s[11]-s[4]*s[3]*s[9] -s[8]*s[1]*s[7]  +s[8]*s[3]*s[5];
        tmp[15] =  s[0]*s[5]*s[10] -s[0]*s[6]*s[9]  -s[4]*s[1]*s[10]+s[4]*s[2]*s[9] +s[8]*s[1]*s[6]  -s[8]*s[2]*s[5];
        double det = s[0]*tmp[0]+s[1]*tmp[4]+s[2]*tmp[8]+s[3]*tmp[12];
        if (std::fabs(det) < 1e-12) return identity();
        double id = 1.0 / det;
        Mat4 r;
        for (int i = 0; i < 16; i++) (&r.m[0][0])[i] = tmp[i] * id;
        return r;
    }
    point3 apply_point(const point3& p) const {
        return point3(m[0][0]*p.x()+m[0][1]*p.y()+m[0][2]*p.z()+m[0][3],
                      m[1][0]*p.x()+m[1][1]*p.y()+m[1][2]*p.z()+m[1][3],
                      m[2][0]*p.x()+m[2][1]*p.y()+m[2][2]*p.z()+m[2][3]);
    }
    vec3 apply_vector(const vec3& v) const {
        return vec3(m[0][0]*v.x()+m[0][1]*v.y()+m[0][2]*v.z(),
                    m[1][0]*v.x()+m[1][1]*v.y()+m[1][2]*v.z(),
                    m[2][0]*v.x()+m[2][1]*v.y()+m[2][2]*v.z());
    }
    ray apply_ray(const ray& r) const {
        return ray(apply_point(r.origin()), apply_vector(r.direction()));
    }
};

inline Mat4 mat4_from_transforms(const std::vector<Transform>& transforms) {
    Mat4 M = Mat4::identity();
    for (const auto& t : transforms) {
        Mat4 T;
        switch (t.type) {
            case TransformType::Translate: T = Mat4::translate(t.x, t.y, t.z); break;
            case TransformType::Scale:     T = Mat4::scale(t.x, t.y, t.z);     break;
            case TransformType::RotateX:   T = Mat4::rotateX(t.theta);          break;
            case TransformType::RotateY:   T = Mat4::rotateY(t.theta);          break;
            case TransformType::RotateZ:   T = Mat4::rotateZ(t.theta);          break;
        }
        M = M * T;
    }
    return M;
}

#endif