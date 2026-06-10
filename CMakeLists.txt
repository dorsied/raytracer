cmake_minimum_required(VERSION 3.10)
project(raytracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release build by default; override with -DCMAKE_BUILD_TYPE=Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -Wextra")

# ── Source files ──────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/parser/SceneParser.cpp
    external/tinyxml2/tinyxml2.cpp
    # Add more as you implement them:
    # src/geometry/Sphere.cpp
    # src/geometry/Mesh.cpp
)

add_executable(raytracer ${SOURCES})

target_include_directories(raytracer PRIVATE
    src
    external
)
