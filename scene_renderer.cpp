#include "scene_renderer.h"
#include <cmath>
#include <iostream>
#include <limits>
using namespace parser;

constexpr const float kEpsilon = 1e-6;

inline bool SameSide(const Vec3f &point, const Vec3f &vertex_c,
                     const Vec3f &vertex_a, const Vec3f &vertex_b) {
  const Vec3f vec_ba = vertex_b - vertex_a;
  const Vec3f vec_pa = point - vertex_a;
  const Vec3f vec_ca = vertex_c - vertex_a;
  return (vec_ba.CrossProduct(vec_pa)) * (vec_ba.CrossProduct(vec_ca)) >= 0.0f;
}

float SceneRenderer::DoesIntersect(const Vec3f &origin, const Vec3f &distance,
                                   const Face &face) {
  const Vec3f vertex_0 = scene_.vertex_data[face.v0_id];
  const Vec3f vertex_1 = scene_.vertex_data[face.v1_id];
  const Vec3f vertex_2 = scene_.vertex_data[face.v2_id];
  const Vec3f vertex_to_camera = origin - vertex_0;
  const float t = -(vertex_to_camera * face.normal) / (distance * face.normal);
  const Vec3f point = origin + distance * t;
  if (SameSide(point, vertex_0, vertex_1, vertex_2) &&
      SameSide(point, vertex_1, vertex_0, vertex_2) &&
      SameSide(point, vertex_2, vertex_0, vertex_1)) {
    return t;
  }
  return std::numeric_limits<float>::infinity();
}

float SceneRenderer::DoesIntersect(const Vec3f &origin, const Vec3f &distance,
                                   const Mesh &mesh, Face &intersecting_face) {
  float tmin = std::numeric_limits<float>::infinity();
  for (const Face &face : mesh.faces) {
    const float t = DoesIntersect(origin, distance, face);
    if (t < tmin) {
      tmin = t;
      intersecting_face = face;
    }
  }
  return tmin;
}

float SceneRenderer::DoesIntersect(const Vec3f &origin, const Vec3f &distance,
                                   const Triangle &triangle) {
  return DoesIntersect(origin, distance, triangle.indices);
}

float SceneRenderer::DoesIntersect(const Vec3f &origin, const Vec3f &distance,
                                   const Sphere &sphere) {
  const Vec3f center_of_sphere = scene_.vertex_data[sphere.center_vertex_id];
  const Vec3f sphere_to_camera = origin - center_of_sphere;
  const float distance_times_sphere_to_camera = distance * sphere_to_camera;
  const float norm_of_distance_squared = distance * distance;
  const float norm_of_sphere_to_camera_squared =
      sphere_to_camera * sphere_to_camera;
  const float radius_squared = sphere.radius * sphere.radius;
  const float determinant =
      distance_times_sphere_to_camera * distance_times_sphere_to_camera -
      norm_of_distance_squared *
          (norm_of_sphere_to_camera_squared - radius_squared);
  if (determinant < kEpsilon) {
    return std::numeric_limits<float>::infinity();
  } else if (fabs(determinant) <= kEpsilon) {
    return -distance_times_sphere_to_camera / norm_of_distance_squared;
  } else {
    float t1 = (-distance_times_sphere_to_camera + sqrt(determinant)) /
               norm_of_distance_squared;
    float t2 = (-distance_times_sphere_to_camera - sqrt(determinant)) /
               norm_of_distance_squared;
    return fmin(t1, t2);
  }
}

Vec3f SceneRenderer::CalculateS(int i, int j, const Camera &camera) {
  const Vec4f view_plane = camera.near_plane;
  const Vec3f gaze = camera.gaze;
  const float dist = camera.near_distance;
  const float l = view_plane.x;
  const float r = view_plane.y;
  const float b = view_plane.z;
  const float t = view_plane.w;
  const Vec3f v = camera.up;
  const Vec3f u = gaze.CrossProduct(v);

  const Vec3f m = camera.position + gaze * dist;
  const Vec3f q = m + u * l + v * t;

  const float su = (r - l) * (i + .5) / camera.image_width;
  const float sv = (t - b) * (j + .5) / camera.image_height;

  return q + u * su - v * sv;
}

Vec3i SceneRenderer::RenderPixel(int i, int j, const Camera &camera) {
  // TODO(kadircet): Should we clamp at each step, or only at the final
  // stage?
  Vec3f color = scene_.background_color;
  int material_id = -1;
  Vec3f normal;
  float tmin = std::numeric_limits<float>::infinity();
  const Vec3f origin = camera.position;
  const Vec3f distance = CalculateS(i, j, camera) - origin;

  for (const Triangle &obj : scene_.triangles) {
    float t = DoesIntersect(origin, distance, obj);
    if (t < tmin && t > 0.0f) {
      tmin = t;
      material_id = obj.material_id;
      normal = obj.indices.normal;
    }
  }
  for (const Sphere &obj : scene_.spheres) {
    float t = DoesIntersect(origin, distance, obj);
    if (t < tmin && t > 0.0f) {
      tmin = t;
      material_id = obj.material_id;
      normal = distance * t - scene_.vertex_data[obj.center_vertex_id];
      normal /= obj.radius;
    }
  }
  parser::Face face;
  for (const Mesh &obj : scene_.meshes) {
    float t = DoesIntersect(origin, distance, obj, face);
    if (t < tmin && t > 0.0f) {
      tmin = t;
      material_id = obj.material_id;
      normal = face.normal;
    }
  }

  if (material_id != -1) {
    const Material material = scene_.materials[material_id];
    color = scene_.ambient_light.PointWise(material.ambient);

    const Vec3f intersection_point = origin + distance * tmin;
    Vec3f w0 = origin - intersection_point;
    //TODO: should w0 be normalized?   
    //In slides 3 w0 is the unit vector from s to e
    //Check this @kadircet
    w0.Normalize();
    if (w0 * normal < .0) {
      normal *= -1;
    }
    for (const PointLight &light : scene_.point_lights) {
      
      //Shadow check  
      float tmin_shadow = std::numeric_limits<float>::infinity();
      bool shadow_exists = false;
      Vec3f wi = light.position - intersection_point; 
      const Vec3f intersection_point_with_epsilon = intersection_point + 
          (wi * scene_.shadow_ray_epsilon);
      for (const Triangle &obj : scene_.triangles) {
        float t = DoesIntersect(intersection_point_with_epsilon, wi, obj);
        if (t < tmin_shadow && t > 0.0f) {
          shadow_exists = true;
          break;
        }
      }
      if (shadow_exists) {
        continue;
      }
      for (const Sphere &obj : scene_.spheres) {
        float t = DoesIntersect(intersection_point_with_epsilon, wi, obj);
        if (t < tmin_shadow && t > 0.0f) {
          shadow_exists = true;
          break;
        }
      }
      if (shadow_exists) {
        continue;
      }
      parser::Face face;
      for (const Mesh &obj : scene_.meshes) {
        float t = DoesIntersect(intersection_point_with_epsilon, wi, obj, face);
        if (t < tmin_shadow && t > 0.0f) {
          shadow_exists = true;
          break;
        }
      }
      if (shadow_exists) {
        continue;
      }
      // 

      const float r_square = wi * wi;
      wi.Normalize();

      Vec3f h = wi + w0;
      h.Normalize();

      const Vec3f intensity = light.intensity / r_square;
      const float cos_theta = wi * normal;
      const float cos_thetap = cos_theta > 0. ? cos_theta : 0.;
      color += (material.diffuse * cos_thetap).PointWise(intensity);

      const float cos_alpha = normal * h;
      const float cos_alphap = cos_alpha > 0. ? cos_alpha : 0.;
      color += (material.specular * pow(cos_alphap, material.phong_exponent))
                   .PointWise(intensity);
    }
  }

  return color.ToVec3i();
}

Vec3i *SceneRenderer::RenderImage(const Camera &camera) {
  const int width = camera.image_width;
  const int height = camera.image_height;
  Vec3i *result = new Vec3i[width * height];

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      result[j * width + i] = RenderPixel(i, j, camera);
    }
  }

  return result;
}
