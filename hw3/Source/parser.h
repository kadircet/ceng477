#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <math.h>
#include <string>
#include <vector>
#include "vector.h"

namespace parser {
struct Camera {
  Vec3f position;
  Vec3f gaze;
  Vec3f up;
  Vec4f near_plane;
  float near_distance;
  float far_distance;
  int image_width, image_height;
};

struct PointLight {
  Vec3f position;
  Vec3f intensity;
  bool status;
};

struct Material {
  Vec3f ambient;
  Vec3f diffuse;
  Vec3f specular;
  float phong_exponent;
};

struct Transformation {
  std::string transformation_type;
  int id;
};

struct Face {
  int v0_id;
  int v1_id;
  int v2_id;
  Vec3f normal;

  void CalculateNormal(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2) {
    const Vec3f ba = v0 - v1;
    const Vec3f ca = v0 - v2;
    normal = ba.CrossProduct(ca).Normalized();
  }
};

struct Mesh {
  int material_id;
  std::vector<Face> faces;
  std::vector<Transformation> transformations;
  std::string mesh_type;
};

struct Scene {
  // Data
  Vec3i background_color;
  Camera camera;
  Vec3f ambient_light;
  std::vector<PointLight> point_lights;
  std::vector<Material> materials;
  std::vector<Vec3f> vertex_data;
  std::vector<Vec3f> translations;
  std::vector<Vec3f> scalings;
  std::vector<Vec4f> rotations;
  std::vector<Mesh> meshes;
  uint32_t face_count;

  // Functions
  void loadFromXml(const std::string& filepath);
};
}  // namespace parser

#endif
