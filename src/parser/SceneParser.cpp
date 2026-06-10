#include "SceneParser.h"
#include "../../external/tinyxml2/tinyxml2.h"
#include <cstring>
#include <string>

using namespace tinyxml2;

// helpers 

static float requireFloat(const XMLElement* el, const char* attr) {
    float v = 0;
    if (el->QueryFloatAttribute(attr, &v) != XML_SUCCESS)
        throw ParseError(std::string("Missing/bad float attribute '") + attr +
                         "' on <" + el->Name() + ">");
    return v;
}

static int requireInt(const XMLElement* el, const char* attr) {
    int v = 0;
    if (el->QueryIntAttribute(attr, &v) != XML_SUCCESS)
        throw ParseError(std::string("Missing/bad int attribute '") + attr +
                         "' on <" + el->Name() + ">");
    return v;
}

static const char* requireStr(const XMLElement* el, const char* attr) {
    const char* v = el->Attribute(attr);
    if (!v)
        throw ParseError(std::string("Missing string attribute '") + attr +
                         "' on <" + el->Name() + ">");
    return v;
}

static const XMLElement* requireChild(const XMLElement* parent, const char* name) {
    const XMLElement* child = parent->FirstChildElement(name);
    if (!child)
        throw ParseError(std::string("Expected <") + name +
                         "> inside <" + parent->Name() + ">");
    return child;
}

// sub-parsers

static Vec3 parseVec3(const XMLElement* el) {
    return { requireFloat(el, "x"), requireFloat(el, "y"), requireFloat(el, "z") };
}

static Color parseColor(const XMLElement* el) {
    return { requireFloat(el, "r"), requireFloat(el, "g"), requireFloat(el, "b") };
}

static PhongParams parsePhong(const XMLElement* el) {
    PhongParams p;
    p.ka = requireFloat(el, "ka");
    p.kd = requireFloat(el, "kd");
    p.ks = requireFloat(el, "ks");
    p.exponent = requireFloat(el, "exponent");
    return p;
}

static SceneMaterial parseMaterialSolid(const XMLElement* el) {
    SceneMaterial m;
    m.textured = false;
    m.color = parseColor(requireChild(el, "color"));
    m.phong = parsePhong(requireChild(el, "phong"));
    m.reflectance = requireFloat(requireChild(el, "reflectance"),  "r");
    m.transmittance = requireFloat(requireChild(el, "transmittance"), "t");
    m.ior = requireFloat(requireChild(el, "refraction"),    "iof");
    return m;
}

static SceneMaterial parseMaterialTextured(const XMLElement* el) {
    SceneMaterial m;
    m.textured = true;
    m.textureName = requireStr(requireChild(el, "texture"), "name");
    m.phong = parsePhong(requireChild(el, "phong"));
    m.reflectance = requireFloat(requireChild(el, "reflectance"),  "r");
    m.transmittance = requireFloat(requireChild(el, "transmittance"), "t");
    m.ior = requireFloat(requireChild(el, "refraction"),    "iof");
    return m;
}

// Read either material_solid or material_textured from an element.
static SceneMaterial parseMaterial(const XMLElement* parent) {
    if (auto* el = parent->FirstChildElement("material_solid"))
        return parseMaterialSolid(el);
    if (auto* el = parent->FirstChildElement("material_textured"))
        return parseMaterialTextured(el);
    throw ParseError(std::string("No material found inside <") + parent->Name() + ">");
}

// parse all transforms inside a <transform> block
static std::vector<Transform> parseTransforms(const XMLElement* parent) {
    std::vector<Transform> result;
    const XMLElement* tblock = parent->FirstChildElement("transform");
    if (!tblock) return result;

    for (const XMLElement* t = tblock->FirstChildElement(); t; t = t->NextSiblingElement()) {
        const char* name = t->Name();
        Transform tr;
        if (std::strcmp(name, "translate") == 0) {
            tr.type = TransformType::Translate;
            tr.x = requireFloat(t, "x");
            tr.y = requireFloat(t, "y");
            tr.z = requireFloat(t, "z");
        } else if (std::strcmp(name, "scale") == 0) {
            tr.type = TransformType::Scale;
            tr.x = requireFloat(t, "x");
            tr.y = requireFloat(t, "y");
            tr.z = requireFloat(t, "z");
        } else if (std::strcmp(name, "rotateX") == 0) {
            tr.type  = TransformType::RotateX;
            tr.theta = requireFloat(t, "theta");
        } else if (std::strcmp(name, "rotateY") == 0) {
            tr.type  = TransformType::RotateY;
            tr.theta = requireFloat(t, "theta");
        } else if (std::strcmp(name, "rotateZ") == 0) {
            tr.type  = TransformType::RotateZ;
            tr.theta = requireFloat(t, "theta");
        }
        result.push_back(tr);
    }
    return result;
}

// lights 

static void parseLights(const XMLElement* lightsEl, Scene& scene) {
    for (const XMLElement* el = lightsEl->FirstChildElement(); el; el = el->NextSiblingElement()) {
        const char* tag = el->Name();

        if (std::strcmp(tag, "ambient_light") == 0) {
            if (scene.hasAmbient)
                throw ParseError("More than one <ambient_light> found");
            scene.ambientLight.color = parseColor(requireChild(el, "color"));
            scene.hasAmbient = true;

        } else if (std::strcmp(tag, "parallel_light") == 0) {
            ParallelLight l;
            l.color = parseColor(requireChild(el, "color"));
            l.direction = parseVec3(requireChild(el, "direction"));
            scene.parallelLights.push_back(l);

        } else if (std::strcmp(tag, "point_light") == 0) {
            PointLight l;
            l.color = parseColor(requireChild(el, "color"));
            l.position = parseVec3(requireChild(el, "position"));
            scene.pointLights.push_back(l);

        } else if (std::strcmp(tag, "spot_light") == 0) {
            SpotLight l;
            l.color = parseColor(requireChild(el, "color"));
            l.position = parseVec3(requireChild(el, "position"));
            l.direction = parseVec3(requireChild(el, "direction"));
            const auto* fo = requireChild(el, "falloff");
            l.alpha1 = requireFloat(fo, "alpha1");
            l.alpha2 = requireFloat(fo, "alpha2");
            scene.spotLights.push_back(l);
        }
    }
}

// surfaces

static void parseSurfaces(const XMLElement* surfacesEl, Scene& scene) {
    for (const XMLElement* el = surfacesEl->FirstChildElement(); el; el = el->NextSiblingElement()) {
        const char* tag = el->Name();

        if (std::strcmp(tag, "sphere") == 0) {
            Sphere s;
            s.radius = requireFloat(el, "radius");
            s.position = parseVec3(requireChild(el, "position"));
            s.material = parseMaterial(el);
            s.transforms = parseTransforms(el);
            scene.spheres.push_back(s);

        } else if (std::strcmp(tag, "mesh") == 0) {
            Mesh m;
            m.name = requireStr(el, "name");
            m.material = parseMaterial(el);
            m.transforms = parseTransforms(el);
            scene.meshes.push_back(m);
        }
    }
}

// camera

static Camera parseCamera(const XMLElement* camEl) {
    Camera cam;
    cam.position = parseVec3(requireChild(camEl, "position"));
    cam.lookat = parseVec3(requireChild(camEl, "lookat"));
    cam.up = parseVec3(requireChild(camEl, "up"));
    cam.hfov = requireFloat(requireChild(camEl, "horizontal_fov"), "angle");
    const auto* res = requireChild(camEl, "resolution");
    cam.resH = requireInt(res, "horizontal");
    cam.resV = requireInt(res, "vertical");
    cam.maxBounces = requireInt(requireChild(camEl, "max_bounces"), "n");
    cam.transforms = parseTransforms(camEl);
    return cam;
}

// public entry point

Scene SceneParser::parse(const std::string& path) {
    XMLDocument doc;
    XMLError err = doc.LoadFile(path.c_str());
    if (err != XML_SUCCESS)
        throw std::runtime_error("Cannot open/parse XML: " + path +
                                 " (" + XMLDocument::ErrorIDToName(err) + ")");

    const XMLElement* root = doc.FirstChildElement("scene");
    if (!root)
        throw ParseError("Root <scene> element not found");

    Scene scene;

    // output_file
    if (const char* of = root->Attribute("output_file"))
        scene.outputFile = of;

    // background_color
    if (const XMLElement* bg = root->FirstChildElement("background_color"))
        scene.background = parseColor(bg);

    // camera
    scene.camera = parseCamera(requireChild(root, "camera"));

    // lights
    if (const XMLElement* lights = root->FirstChildElement("lights"))
        parseLights(lights, scene);

    // surfaces
    if (const XMLElement* surfaces = root->FirstChildElement("surfaces"))
        parseSurfaces(surfaces, scene);

    return scene;
}
