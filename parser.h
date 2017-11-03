#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "bounding_volume_hierarchy.h"
#include "vector.h"

namespace parser {
namespace {

float Determinant(Vec3f a, Vec3f b, Vec3f c) {
  // vectors are columns
  return a.x * (b.y * c.z - c.y * b.z) + a.y * (c.x * b.z - b.x * c.z) +
         a.z * (b.x * c.y - c.x * b.y);
}
}  // namespace

struct Camera {
  Vec3f position;
  Vec3f gaze;
  Vec3f up;
  Vec4f near_plane;
  float near_distance;
  int image_width, image_height;
  std::string image_name;
};

struct PointLight {
  Vec3f position;
  Vec3f intensity;
};

struct Material {
  Vec3f ambient;
  Vec3f diffuse;
  Vec3f specular;
  Vec3f mirror;
  float phong_exponent;
};

struct Face : Object {
  int v0_id;
  int v1_id;
  int v2_id;
  int material_id;
  Vec3f normal;

  void SayMyName() const { std::cout << "Face" << v0_id << std::endl; }

  void CalculateNormal(const std::vector<Vec3f>& vertex_data) {
    const Vec3f v0 = vertex_data[v0_id];
    const Vec3f e1 = vertex_data[v1_id] - v0;
    const Vec3f e2 = vertex_data[v2_id] - v0;
    normal = e1.CrossProduct(e2).Normalized();
  }

  HitRecord GetIntersection(const Ray& ray, const Scene& scene) const {
    HitRecord hit_record;
    hit_record.t = kInf;
    hit_record.material_id = -1;
    const std::vector<Vec3f> vertex_data = scene.vertex_data;
    const Vec3f direction = ray.direction;
    const Vec3f vertex_0 = vertex_data[v0_id];
    const Vec3f vertex_1 = vertex_data[v1_id];
    const Vec3f vertex_2 = vertex_data[v2_id];
    // a->v0 b->v1 c->v2
    const Vec3f ba = vertex_0 - vertex_1;
    const Vec3f ca = vertex_0 - vertex_2;
    const float detA = Determinant(ba, ca, direction);
    const Vec3f oa = (vertex_0 - ray.origin) / detA;
    const float beta = Determinant(oa, ca, direction);
    if (beta < .0f) {
      return hit_record;
    }
    const float gama = Determinant(ba, oa, direction);
    if (gama < .0f || beta + gama > 1.0f) {
      return hit_record;
    }
    const float t = Determinant(ba, ca, oa);
    if (t >= 0 && t <= std::numeric_limits<float>::infinity()) {
      hit_record.t = t;
      hit_record.normal = normal;
      hit_record.material_id = material_id;
      hit_record.obj = this;
    }
    return hit_record;
  }

  BoundingBox GetBoundingBox(const Scene& scene) const {
    if (bounding_box == nullptr) {
      const std::vector<Vec3f> vertex_data = scene.vertex_data;
      Vec3f min_c = vertex_data[v0_id];
      Vec3f max_c = vertex_data[v0_id];

      min_c.x = fmin(min_c.x, vertex_data[v1_id].x);
      min_c.y = fmin(min_c.y, vertex_data[v1_id].y);
      min_c.z = fmin(min_c.z, vertex_data[v1_id].z);

      max_c.x = fmax(max_c.x, vertex_data[v1_id].x);
      max_c.y = fmax(max_c.y, vertex_data[v1_id].y);
      max_c.z = fmax(max_c.z, vertex_data[v1_id].z);

      min_c.x = fmin(min_c.x, vertex_data[v2_id].x);
      min_c.y = fmin(min_c.y, vertex_data[v2_id].y);
      min_c.z = fmin(min_c.z, vertex_data[v2_id].z);

      max_c.x = fmax(max_c.x, vertex_data[v2_id].x);
      max_c.y = fmax(max_c.y, vertex_data[v2_id].y);
      max_c.z = fmax(max_c.z, vertex_data[v2_id].z);
      bounding_box = std::make_unique<BoundingBox>(min_c, max_c);
    }

    return *bounding_box;
  }

 private:
  mutable std::unique_ptr<BoundingBox> bounding_box;
};  // namespace parser

struct Mesh {
  int material_id;
  std::vector<Face> faces;
};

struct Triangle {
  int material_id;
  Face indices;
};

struct Sphere : Object {
  int material_id;
  int center_vertex_id;
  float radius;

  void SayMyName() const { std::cout << "Sphere" << std::endl; }

  Vec3f GetNormal(float t, const Ray& ray, const Vec3f& center) const {
    return (ray.direction * t + ray.origin - center).Normalized();
  }

  HitRecord GetIntersection(const Ray& ray, const Scene& scene) const {
    HitRecord hit_record;
    hit_record.material_id = -1;
    const Vec3f center_of_sphere = scene.vertex_data[center_vertex_id];
    const Vec3f sphere_to_camera = ray.origin - center_of_sphere;
    const float direction_times_sphere_to_camera =
        ray.direction * sphere_to_camera;
    const float norm_of_sphere_to_camera_squared =
        sphere_to_camera * sphere_to_camera;
    const float radius_squared = radius * radius;
    const float determinant =
        direction_times_sphere_to_camera * direction_times_sphere_to_camera -
        (norm_of_sphere_to_camera_squared - radius_squared);
    hit_record.t = std::numeric_limits<float>::infinity();
    if (determinant < -kEpsilon) {
      return hit_record;
    } else if (determinant < kEpsilon) {
      hit_record.t = -direction_times_sphere_to_camera;
    } else {
      const float t1 = (-direction_times_sphere_to_camera + sqrt(determinant));
      const float t2 = (-direction_times_sphere_to_camera - sqrt(determinant));
      hit_record.t = fmin(t1, t2);
    }
    hit_record.material_id = material_id;
    hit_record.normal = GetNormal(hit_record.t, ray, center_of_sphere);
    hit_record.obj = this;
    return hit_record;
  }

  BoundingBox GetBoundingBox(const Scene& scene) const {
    if (bounding_box == nullptr) {
      const Vec3f vertex_data = scene.vertex_data[center_vertex_id];
      const Vec3f rad_vec = Vec3f(radius, radius, radius);
      const Vec3f min_c = vertex_data - rad_vec;
      const Vec3f max_c = vertex_data + rad_vec;
      bounding_box = std::make_unique<BoundingBox>(min_c, max_c);
    }

    return *bounding_box;
  }

 private:
  mutable std::unique_ptr<BoundingBox> bounding_box;
};

}  // namespace parser

#endif
