#ifndef MESH_H
#define MESH_H

#include "hittable_list.h"
#include "triangles.h"
#include "material.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

inline hittable_list load_obj(const std::string& path, const Material& mat) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open OBJ file: " + path);

    std::vector<point3> positions;
    std::vector<vec3> normals;
    std::vector<double> tex_u, tex_v;

    hittable_list triangles;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            double x, y, z;
            ss >> x >> y >> z;
            positions.emplace_back(x, y, z);

        } else if (token == "vn") {
            double x, y, z;
            ss >> x >> y >> z;
            normals.emplace_back(x, y, z);

        } else if (token == "vt") {
            double u, v;
            ss >> u >> v;
            tex_u.push_back(u);
            tex_v.push_back(v);

        } else if (token == "f") {
            struct FaceVertex { int vi, ti, ni; };
            std::vector<FaceVertex> verts;

            std::string chunk;
            while (ss >> chunk) {
                FaceVertex fv = {0, 0, 0};
                for (char& ch : chunk) if (ch == '/') ch = ' ';
                std::istringstream cs(chunk);
                cs >> fv.vi;
                if (!(cs >> fv.ti)) fv.ti = 0;
                if (!(cs >> fv.ni)) fv.ni = 0;
                verts.push_back(fv);
            }

            for (int i = 1; i + 1 < (int)verts.size(); i++) {
                auto get_pos = [&](int idx) -> point3 {
                    int i = idx > 0 ? idx - 1 : (int)positions.size() + idx;
                    return positions.at(i);
                };
                auto get_norm = [&](int idx) -> vec3 {
                    if (idx == 0) return vec3(0, 1, 0);   // fallback
                    int i = idx > 0 ? idx - 1 : (int)normals.size() + idx;
                    return normals.at(i);
                };
                auto get_u = [&](int idx) -> double {
                    if (idx == 0) return 0.0;
                    int i = idx > 0 ? idx - 1 : (int)tex_u.size() + idx;
                    return tex_u.at(i);
                };
                auto get_v = [&](int idx) -> double {
                    if (idx == 0) return 0.0;
                    int i = idx > 0 ? idx - 1 : (int)tex_v.size() + idx;
                    return tex_v.at(i);
                };

                const auto& a = verts[0];
                const auto& b = verts[i];
                const auto& c = verts[i + 1];

                triangles.add(make_shared<triangle>(
                    get_pos(a.vi),  get_pos(b.vi),  get_pos(c.vi),
                    get_norm(a.ni), get_norm(b.ni), get_norm(c.ni),
                    get_u(a.ti), get_v(a.ti),
                    get_u(b.ti), get_v(b.ti),
                    get_u(c.ti), get_v(c.ti),
                    mat
                ));
            }
        }
    }

    return triangles;
}

#endif