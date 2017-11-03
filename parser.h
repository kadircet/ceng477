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
  Vec3f v0;
  Vec3f v1;
  Vec3f v2;
  int material_id;
  Vec3f normal;

  void CalculateNormal() {
    ba = v0 - v1;
    ca = v0 - v2;
    normal = ba.CrossProduct(ca).Normalized();

    Vec3f min_c = v0;
    Vec3f max_c = v0;

    min_c.x = fmin(min_c.x, v1.x);
    min_c.y = fmin(min_c.y, v1.y);
    min_c.z = fmin(min_c.z, v1.z);

    max_c.x = fmax(max_c.x, v1.x);
    max_c.y = fmax(max_c.y, v1.y);
    max_c.z = fmax(max_c.z, v1.z);

    min_c.x = fmin(min_c.x, v2.x);
    min_c.y = fmin(min_c.y, v2.y);
    min_c.z = fmin(min_c.z, v2.z);

    max_c.x = fmax(max_c.x, v2.x);
    max_c.y = fmax(max_c.y, v2.y);
    max_c.z = fmax(max_c.z, v2.z);
    bounding_box.min_corner = min_c;
    bounding_box.max_corner = max_c;
  }

  HitRecord GetIntersection(const Ray& ray) const {
    HitRecord hit_record;
    hit_record.t = kInf;
    hit_record.material_id = -1;
    if (!ray.is_shadow && ray.direction * normal > .0) {
      return hit_record;
    }
    const Vec3f direction = ray.direction;
    // a->v0 b->v1 c->v2
    const float detA = Determinant(ba, ca, direction);
    const Vec3f oa = (v0 - ray.origin) / detA;
    const float beta = Determinant(oa, ca, direction);
    if (beta < -kEpsilon) {
      return hit_record;
    }
    const float gama = Determinant(ba, oa, direction);
    if (gama < -kEpsilon || beta + gama > 1.0f + kEpsilon) {
      return hit_record;
    }
    const float t = Determinant(ba, ca, oa);
    if (t > -kEpsilon) {
      hit_record.t = t;
      hit_record.normal = normal;
      hit_record.material_id = material_id;
      hit_record.obj = this;
    }
    return hit_record;
  }

  const BoundingBox GetBoundingBox() const { return bounding_box; }

 private:
  BoundingBox bounding_box;
  Vec3f ba;
  Vec3f ca;
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
  Vec3f center_of_sphere;
  float radius;

  Vec3f GetNormal(float t, const Ray& ray) const {
    return (ray.direction * t + ray.origin - center_of_sphere).Normalized();
  }

  HitRecord GetIntersection(const Ray& ray) const {
    HitRecord hit_record;
    hit_record.material_id = -1;
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
    hit_record.normal = GetNormal(hit_record.t, ray);
    hit_record.obj = this;
    return hit_record;
  }

  void Initialize() {
    const Vec3f rad_vec = Vec3f(radius, radius, radius);
    const Vec3f min_c = center_of_sphere - rad_vec;
    const Vec3f max_c = center_of_sphere + rad_vec;
    bounding_box.min_corner = min_c;
    bounding_box.max_corner = max_c;
  }

  const BoundingBox GetBoundingBox() const { return bounding_box; }

  BoundingBox bounding_box;
};

}  // namespace parser

#endif
