#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "bounding_volume_hierarchy.h"
#include "jpeg.h"
#include "vector.h"

namespace parser {
namespace {

float Determinant(Vec3f a, Vec3f b, Vec3f c) {
  // vectors are columns
  return a.x * (b.y * c.z - c.y * b.z) + a.y * (c.x * b.z - b.x * c.z) +
         a.z * (b.x * c.y - c.x * b.y);
}
}  // namespace

struct Transformation {
  enum TransformationType {
    SCALING,
    TRANSLATION,
    ROTATION,
  };
  TransformationType transformation_type;
  int index;

  static TransformationType GetType(char c) {
    if (c == 's') return SCALING;
    return c == 't' ? TRANSLATION : ROTATION;
  }
};

struct Scaling : Transformation {
  float x, y, z;

  Matrix ToMatrix() {
    Matrix mat;
    mat[0][0] = x;
    mat[1][1] = y;
    mat[2][2] = z;
    mat[3][3] = 1;

    return mat;
  }

  Matrix InverseMatrix() {
    Matrix mat;
    mat[0][0] = 1 / x;
    mat[1][1] = 1 / y;
    mat[2][2] = 1 / z;
    mat[3][3] = 1;

    return mat;
  }
};

struct Translation : Transformation {
  float x, y, z;

  Matrix ToMatrix() {
    Matrix mat;
    mat.MakeIdentity();
    mat[0][3] = x;
    mat[1][3] = y;
    mat[2][3] = z;

    return mat;
  }

  Matrix InverseMatrix() {
    Matrix mat;
    mat.MakeIdentity();
    mat[0][3] = -x;
    mat[1][3] = -y;
    mat[2][3] = -z;
    mat[3][3] = 1;

    return mat;
  }
};

struct Rotation : Transformation {
  float angle, x, y, z;

  Matrix ToMatrix() {
    const Vec3f u = Vec3f(x, y, z).Normalized();
    const Vec3f v = ((x != 0 || y != 0) ? Vec3f(-u.y, u.x, .0) : Vec3f(0, 1, 0))
                        .Normalized();
    const Vec3f w = u.CrossProduct(v);

    Matrix M;
    M[0][0] = u.x;
    M[0][1] = u.y;
    M[0][2] = u.z;
    M[1][0] = v.x;
    M[1][1] = v.y;
    M[1][2] = v.z;
    M[2][0] = w.x;
    M[2][1] = w.y;
    M[2][2] = w.z;
    M[3][3] = 1;

    Matrix rot;
    rot[0][0] = 1;
    rot[1][1] = cos(angle);
    rot[1][2] = -sin(angle);
    rot[2][1] = -rot[1][2];
    rot[2][2] = rot[1][1];
    rot[3][3] = 1;

    return M.Transpose() * (rot * M);
  }

  Matrix InverseMatrix() {
    const Vec3f u = Vec3f(x, y, z).Normalized();
    const Vec3f v = ((x != 0 || y != 0) ? Vec3f(-u.y, u.x, .0) : Vec3f(0, 1, 0))
                        .Normalized();
    const Vec3f w = u.CrossProduct(v);

    Matrix M;
    M[0][0] = u.x;
    M[0][1] = u.y;
    M[0][2] = u.z;
    M[1][0] = v.x;
    M[1][1] = v.y;
    M[1][2] = v.z;
    M[2][0] = w.x;
    M[2][1] = w.y;
    M[2][2] = w.z;
    M[3][3] = 1;

    Matrix rot;
    rot[0][0] = 1;
    rot[1][1] = cos(-angle);
    rot[1][2] = -sin(-angle);
    rot[2][1] = -rot[1][2];
    rot[2][2] = rot[1][1];
    rot[3][3] = 1;

    return M.Transpose() * (rot * M);
  }
};

struct Texture {
  enum InterpolationType {
    NEAREST,
    BILINEAR,
  };
  enum DecalMode {
    REPLACE_KD,
    BLEND_KD,
    REPLACE_ALL,
  };
  enum Appearance {
    REPEAT,
    CLAMP,
  };

  std::string image_name;
  InterpolationType interpolation_type;
  DecalMode decal_mode;
  Appearance appearance;

  static InterpolationType ToInterpolationType(const std::string& str) {
    return str == "nearest" ? NEAREST : BILINEAR;
  }

  static DecalMode ToDecalMode(const std::string& str) {
    if (str == "replace_kd") return REPLACE_KD;
    return str == "blend_kd" ? BLEND_KD : REPLACE_ALL;
  }

  static Appearance ToApperance(const std::string& str) {
    return str == "repeat" ? REPEAT : CLAMP;
  }

  int width;
  int height;
  unsigned char* image_data;

  void LoadImage() {
    read_jpeg_header(image_name.c_str(), width, height);

    image_data = new unsigned char[width * height * 3];
    read_jpeg(image_name.c_str(), image_data, width, height);
  }

  //~Texture() { delete image_data; }

  Vec3f Get(float u, float v) const {
    if (appearance == CLAMP) {
      u = fmax(0., fmin(1., u));
      v = fmax(0., fmin(1., v));
    } else {
      u -= floor(u);
      v -= floor(v);
    }
    u *= width;
    if (u >= width) u--;
    v *= height;
    if (v >= height) v--;
    Vec3f color;
    if (interpolation_type == NEAREST) {
      const unsigned int pos = ((int)v) * width * 3 + ((int)u) * 3;
      color.x = image_data[pos];
      color.y = image_data[pos + 1];
      color.z = image_data[pos + 2];
    } else {
      const unsigned int p = u;
      const unsigned int q = v;
      const float dx = u - p;
      const float dy = v - q;
      const unsigned int pos = q * width * 3 + p * 3;
      color.x = image_data[pos] * (1 - dx) * (1 - dy);
      color.x += image_data[pos + 3] * (dx) * (1 - dy);
      color.x += image_data[pos + 3 + width * 3] * (dx) * (dy);
      color.x += image_data[pos + width * 3] * (1 - dx) * (dy);

      color.y = image_data[pos + 1] * (1 - dx) * (1 - dy);
      color.y += image_data[pos + 3 + 1] * (dx) * (1 - dy);
      color.y += image_data[pos + 3 + width * 3 + 1] * (dx) * (dy);
      color.y += image_data[pos + width * 3 + 1] * (1 - dx) * (dy);

      color.z = image_data[pos + 2] * (1 - dx) * (1 - dy);
      color.z += image_data[pos + 3 + 2] * (dx) * (1 - dy);
      color.z += image_data[pos + 3 + width * 3 + 2] * (dx) * (dy);
      color.z += image_data[pos + width * 3 + 2] * (1 - dx) * (dy);
    }

    if (decal_mode == BLEND_KD || decal_mode == REPLACE_KD) {
      color /= 255.;
    }
    return color;
  }
};

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
  Vec3f ua;
  Vec3f v1;
  Vec3f ub;
  Vec3f v2;
  Vec3f uc;
  int material_id;
  int texture_id;
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
      hit_record.texture_id = texture_id;
      hit_record.obj = this;
      hit_record.u = ua.x + beta * (ub.x - ua.x) + gama * (uc.x - ua.x);
      hit_record.v = ua.y + beta * (ub.y - ua.y) + gama * (uc.y - ua.y);
      hit_record.intersection_point = ray.origin + ray.direction * t;
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
  int texture_id;

  std::vector<Face> faces;
};

struct MeshInstance {
  int material_id;
  int texture_id;
  int base_mesh_id;
  std::vector<Face> faces;
};

struct Triangle {
  int material_id;
  int texture_id;
  Face indices;
};

struct Sphere : Object {
  int material_id;
  int texture_id;
  Matrix transformation;
  Matrix inverse_transformation;
  Matrix inverse_transformation_transpose;
  Vec3f center_of_sphere;
  float radius;
  float radius_squared;

  Vec3f GetNormal(const Vec3f& intersection_point) const {
    return (intersection_point - center_of_sphere).Normalized();
  }

  HitRecord GetIntersection(const Ray& ray) const {
    HitRecord hit_record;
    hit_record.material_id = -1;
    const Vec3f sphere_to_camera = ray.origin - center_of_sphere;
    const float direction_times_sphere_to_camera =
        ray.direction * sphere_to_camera;
    const float norm_of_sphere_to_camera_squared =
        sphere_to_camera * sphere_to_camera;
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
      if (t2 < .0) {
        hit_record.t = t1;
      } else {
        hit_record.t = t2;
      }
    }
    hit_record.material_id = material_id;
    hit_record.texture_id = texture_id;
    hit_record.intersection_point = ray.origin + ray.direction * hit_record.t;
    hit_record.normal = GetNormal(hit_record.intersection_point);
    hit_record.obj = this;
    hit_record.u =
        (-atan2(hit_record.normal.z, hit_record.normal.x) + M_PI) / M_PI / 2;
    hit_record.v = acos(hit_record.normal.y) / M_PI;
    return hit_record;
  }

  void Initialize() {
    const Vec3f rad_vec = Vec3f(radius, radius, radius);
    const Vec3f min_c = center_of_sphere - rad_vec;
    const Vec3f max_c = center_of_sphere + rad_vec;
    radius_squared = radius * radius;
    bounding_box.min_corner = min_c;
    bounding_box.max_corner = max_c;
  }

  const BoundingBox GetBoundingBox() const { return bounding_box; }

  BoundingBox bounding_box;
};

}  // namespace parser

#endif
