#include "scene_renderer.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace parser;

namespace {

bool NotZero(const Vec3f vec) { return vec.x != 0 || vec.y != 0 || vec.z != 0; }

}  // namespace

float SceneRenderer::DoesIntersect(const Vec3f& origin, const Vec3f& direction,
                                   const Face& face) {
  const Vec3f vertex_0 = scene_.vertex_data[face.v0_id];
  const Vec3f vertex_1 = scene_.vertex_data[face.v1_id];
  const Vec3f vertex_2 = scene_.vertex_data[face.v2_id];
  // a->v0 b->v1 c->v2
  const Vec3f ba = vertex_0 - vertex_1;
  const Vec3f ca = vertex_0 - vertex_2;
  const float detA = Determinant(ba, ca, direction);
  const Vec3f oa = (vertex_0 - origin) / detA;
  const float beta = Determinant(oa, ca, direction);
  if (beta < .0f) {
    return std::numeric_limits<float>::infinity();
  }
  const float gama = Determinant(ba, oa, direction);
  if (gama < .0f || beta + gama > 1.0f) {
    return std::numeric_limits<float>::infinity();
  }
  const float t = Determinant(ba, ca, oa);
  if (t >= 0 && t <= std::numeric_limits<float>::infinity()) {
    return t;
  } else {
    return std::numeric_limits<float>::infinity();
  }
}

float SceneRenderer::DoesIntersect(const Vec3f& origin, const Vec3f& direction,
                                   const Mesh& mesh, float tmax,
                                   const void* hit_obj) {
  for (const Face& face : mesh.faces) {
    if (&face == hit_obj) continue;
    const float t = DoesIntersect(origin, direction, face);
    if (t < tmax && t > .0) {
      return t;
    }
  }
  return std::numeric_limits<float>::infinity();
}

float SceneRenderer::DoesIntersect(const Vec3f& origin, const Vec3f& direction,
                                   const Mesh& mesh,
                                   Face const** intersecting_face) {
  float tmin = std::numeric_limits<float>::infinity();
  for (const Face& face : mesh.faces) {
    const float t = DoesIntersect(origin, direction, face);
    if (t < tmin && t > .0 && face.normal * direction < .0) {
      tmin = t;
      *intersecting_face = &face;
    }
  }
  return tmin;
}

float SceneRenderer::DoesIntersect(const Vec3f& origin, const Vec3f& direction,
                                   const Triangle& triangle) {
  return DoesIntersect(origin, direction, triangle.indices);
}

float SceneRenderer::DoesIntersect(const Vec3f& origin, const Vec3f& direction,
                                   const Sphere& sphere) {
  const Vec3f center_of_sphere = scene_.vertex_data[sphere.center_vertex_id];
  const Vec3f sphere_to_camera = origin - center_of_sphere;
  const float direction_times_sphere_to_camera = direction * sphere_to_camera;
  const float norm_of_sphere_to_camera_squared =
      sphere_to_camera * sphere_to_camera;
  const float radius_squared = sphere.radius * sphere.radius;
  const float determinant =
      direction_times_sphere_to_camera * direction_times_sphere_to_camera -
      (norm_of_sphere_to_camera_squared - radius_squared);
  if (determinant < -kEpsilon) {
    return std::numeric_limits<float>::infinity();
  } else if (determinant < kEpsilon) {
    return -direction_times_sphere_to_camera;
  } else {
    const float sqrt_determinant = sqrt(determinant);
    const float t1 = (-direction_times_sphere_to_camera + sqrt_determinant);
    const float t2 = (-direction_times_sphere_to_camera - sqrt_determinant);
    return fmin(t1, t2);
  }
}

Vec3f SceneRenderer::CalculateS(int i, int j) {
  return q + usu * (i + .5) - vsv * (j + .5);
}

bool SceneRenderer::DoesIntersect(const Ray& ray, float tmax,
                                  const void* hit_obj) {
  const Vec3f origin = ray.origin;
  const Vec3f direction = ray.direction;

  for (const Triangle& obj : scene_.triangles) {
    if (&obj == hit_obj) continue;
    const float t = DoesIntersect(origin, direction, obj);
    if (t < tmax + kEpsilon && t > 0.0f) {
      return true;
    }
  }
  for (const Sphere& obj : scene_.spheres) {
    if (&obj == hit_obj) continue;
    const float t = DoesIntersect(origin, direction, obj);
    if (t < tmax + kEpsilon && t > 0.0f) {
      return true;
    }
  }
  for (const Mesh& obj : scene_.meshes) {
    const float t = DoesIntersect(origin, direction, obj, tmax, hit_obj);
    if (t < tmax + kEpsilon && t > 0.0f) {
      return true;
    }
  }
  return false;
}

Vec3f SceneRenderer::TraceRay(const Ray& ray, int depth,
                              const Object* hit_obj = nullptr) {
  Vec3f color = scene_.background_color;
  const HitRecord hit_record =
      bounding_volume_hierarchy->GetIntersection(ray, hit_obj);
  const int material_id = hit_record.material_id;

  if (material_id != -1) {
    const Vec3f origin = ray.origin;
    const Vec3f intersection_point = origin + ray.direction * hit_record.t;
    const Vec3f normal = hit_record.normal;
    const Material material = scene_.materials[material_id];
    color = scene_.ambient_light.PointWise(material.ambient);

    for (const PointLight& light : scene_.point_lights) {
      // Shadow check
      const Vec3f wi = light.position - intersection_point;
      const Vec3f wi_normal = wi.Normalized();
      const Ray shadow_ray{
          intersection_point + wi_normal * scene_.shadow_ray_epsilon,
          wi_normal};
      // std::cout << "|wi|=" << wi.Length() << std::endl;
      if (bounding_volume_hierarchy->GetIntersection(
              shadow_ray, wi.Length() - scene_.shadow_ray_epsilon,
              hit_record.obj)) {
        continue;
      }

      const float r_square = wi * wi;
      const Vec3f intensity = light.intensity / r_square;

      // Diffuse light
      const float cos_theta = wi_normal * normal;
      const float cos_thetap = cos_theta > 0. ? cos_theta : 0.;
      color += (material.diffuse * cos_thetap).PointWise(intensity);

      // Specular light
      const Vec3f h = (wi_normal - ray.direction).Normalized();
      const float cos_alpha = normal * h;
      const float cos_alphap = cos_alpha > 0. ? cos_alpha : 0.;
      color += (material.specular * pow(cos_alphap, material.phong_exponent))
                   .PointWise(intensity);
    }
    // Specular reflection
    if (depth > 0 && NotZero(material.mirror)) {
      const Vec3f wi =
          (ray.direction + normal * -2 * (ray.direction * normal)).Normalized();
      const Ray reflection_ray{
          intersection_point + wi * scene_.shadow_ray_epsilon, wi};
      color += TraceRay(reflection_ray, depth - 1, hit_record.obj)
                   .PointWise(material.mirror);
    }
  }
  return color;
}

Vec3i SceneRenderer::RenderPixel(int i, int j, const Camera& camera) {
  const Vec3f origin = camera.position;
  const Vec3f direction = (CalculateS(i, j) - origin).Normalized();
  const Ray ray{origin, direction};
  return SceneRenderer::TraceRay(ray, scene_.max_recursion_depth).ToVec3i();
}

Vec3i* SceneRenderer::RenderImage(const Camera& camera) {
  const int width = camera.image_width;
  const int height = camera.image_height;
  Vec3i* result = new Vec3i[width * height];
  const Vec4f view_plane = camera.near_plane;
  const Vec3f gaze = camera.gaze.Normalized();
  const float dist = camera.near_distance;
  const float l = view_plane.x;
  const float r = view_plane.y;
  const float b = view_plane.z;
  const float t = view_plane.w;
  const Vec3f u = gaze.CrossProduct(camera.up).Normalized();
  const Vec3f v = u.CrossProduct(gaze);
  const Vec3f m = camera.position + gaze * dist;
  q = m + u * l + v * t;
  usu = u * (r - l) / camera.image_width;
  vsv = v * (t - b) / camera.image_height;

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      result[j * width + i] = RenderPixel(i, j, camera);
    }
  }

  return result;
}

SceneRenderer::SceneRenderer(const char* scene_path) {
  scene_.loadFromXml(scene_path);
  for (Triangle& obj : scene_.triangles) {
    objects_.push_back(&obj.indices);
  }
  for (Sphere& obj : scene_.spheres) {
    objects_.push_back(&obj);
  }
  for (Mesh& mesh : scene_.meshes) {
    for (Face& obj : mesh.faces) {
      objects_.push_back(&obj);
    }
  }
  bounding_volume_hierarchy = new BoundingVolumeHierarchy(&objects_, &scene_);
  std::cout << "TREE BUILT" << std::endl;
}
