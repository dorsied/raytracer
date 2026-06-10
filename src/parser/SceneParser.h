#pragma once
#include "../core/Scene.h"
#include <string>
#include <stdexcept>

struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class SceneParser {
public:
    static Scene parse(const std::string& path);
};
