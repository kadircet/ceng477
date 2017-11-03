#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

constexpr const float kInf = std::numeric_limits<float>::infinity();
constexpr const float kEpsilon = 1e-6;

namespace parser {
// Notice that all the structures are as simple as possible
// so that you are not enforced to adopt any style or design.
namespace {
int max(int a, int b) { return a > b ? a : b; }

int min(int a, int b) { return a < b ? a : b; }

int round(float a) { return (int)(a > 0 ? a + .5 : a - .5); }

}  // namespace

struct Vec3i {
  int x, y, z;
};

struct Vec3f {
  float x, y, z;

  Vec3f() {}

  Vec3f(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec3f(const Vec3i rhs) {
    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
  }

  Vec3f operator+(const Vec3f rhs) const {
    return Vec3f(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  Vec3f operator-(const Vec3f rhs) const {
    return Vec3f(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  // Dot product.
  float operator*(const Vec3f rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  Vec3f operator*(float rhs) const { return Vec3f(x * rhs, y * rhs, z * rhs); }

  Vec3f operator/(float rhs) const {
    Vec3f result(x / rhs, y / rhs, z / rhs);
    return result;
  }

  Vec3f& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  Vec3f& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Vec3f& operator+=(const Vec3f rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vec3f CrossProduct(const Vec3f rhs) const {
    return Vec3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                 x * rhs.y - y * rhs.x);
  }

  Vec3f PointWise(const Vec3f rhs) const {
    return Vec3f(x * rhs.x, y * rhs.y, z * rhs.z);
  }

  float Length() const { return sqrt(*this * *this); }

  const Vec3f Normalized() const { return *this / this->Length(); }

  Vec3i ToVec3i() const {
    Vec3i res;
    res.x = min(255, max(0, round(x)));
    res.y = min(255, max(0, round(y)));
    res.z = min(255, max(0, round(z)));
    return res;
  }

  float operator[](int idx) const {
    switch (idx) {
      case 0:
        return x;
      case 1:
        return y;
    }
    return z;
  }

  void Print() const { std::cout << x << ' ' << y << ' ' << z << std::endl; }
};

struct Vec4f {
  float x, y, z, w;
};

struct Camera;
struct PointLight;
struct Material;
struct Mesh;
struct Triangle;
struct Sphere;

struct Scene {
  // Data
  Vec3i background_color;
  float shadow_ray_epsilon;
  int max_recursion_depth;
  std::vector<Camera> cameras;
  Vec3f ambient_light;
  std::vector<PointLight> point_lights;
  std::vector<Material> materials;
  std::vector<Vec3f> vertex_data;
  std::vector<Mesh> meshes;
  std::vector<Triangle> triangles;
  std::vector<Sphere> spheres;

  // Functions
  void loadFromXml(const std::string& filepath);
};

}  // namespace parser

#endif
