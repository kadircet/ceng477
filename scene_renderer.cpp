#include <limits>
#include <iostream>
#include <cmath>
#include "scene_renderer.h"
#include <iostream>
using namespace parser;

constexpr const Vec3i red{255,0,0};
inline bool SameSide(const Vec3f& point, const Vec3f& vertex_c, 
            const Vec3f& vertex_a, const Vec3f& vertex_b)
{
    const Vec3f vec_ba = vertex_b - vertex_a; 
    const Vec3f vec_pa = point - vertex_a;
    const Vec3f vec_ca = vertex_c - vertex_a;
    return (vec_ba.CrossProduct(vec_pa))*(vec_ba.CrossProduct(vec_ca)) >= 0.0f;
}
float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Face& face) {
    const Vec3f distance = s - e;
    const Vec3f vertex_0 = scene_.vertex_data[face.v0_id];
    const Vec3f vertex_1 = scene_.vertex_data[face.v1_id];
    const Vec3f vertex_2 = scene_.vertex_data[face.v2_id];
    const Vec3f vertex_to_camera = e - vertex_0;
    const float t = -(vertex_to_camera*face.normal)/(distance*face.normal);
    const Vec3f point = e + (s - e) * t;
    if( SameSide(point, vertex_0, vertex_1, vertex_2) &&
        SameSide(point, vertex_1, vertex_0, vertex_2) &&
        SameSide(point, vertex_2, vertex_0, vertex_1)) {
        return t;
    }
    return std::numeric_limits<float>::infinity();
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Mesh& mesh) {
  float tmin = std::numeric_limits<float>::infinity();
  for(const Face& face : mesh.faces) {
    const float t = DoesIntersect(e, s, face);
    if(t<tmin) {
      tmin = t;
    }
  }
  return tmin;
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Triangle& triangle) {
  return DoesIntersect(e, s, triangle.indices);
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Sphere& sphere) {
    const Vec3f distance = s - e;
    const Vec3f center_of_sphere = scene_.vertex_data[sphere.center_vertex_id];
    const Vec3f sphere_to_camera = e - center_of_sphere;
    const float distance_times_sphere_to_camera = distance * sphere_to_camera;
    const float norm_of_distance_squared = distance * distance;
    const float norm_of_sphere_to_camera_squared = sphere_to_camera * sphere_to_camera;
    const float radius_squared = sphere.radius * sphere.radius;  
    const float determinant = distance_times_sphere_to_camera * distance_times_sphere_to_camera - norm_of_distance_squared * (norm_of_sphere_to_camera_squared - radius_squared); 
    if(determinant < 0.0f) {
        return std::numeric_limits<float>::infinity();
    }
    else if(determinant == 0.0f) {
        return -distance_times_sphere_to_camera / norm_of_distance_squared;
    }
    else 
    {
        float t1 = (-distance_times_sphere_to_camera + sqrt(determinant)) / norm_of_distance_squared;
        float t2 = (-distance_times_sphere_to_camera - sqrt(determinant)) / norm_of_distance_squared;
        return fmin(t1,t2);
    }
}

Vec3f SceneRenderer::CalculateS(int i, int j, const Camera& camera) {
  const Vec4f view_plane = camera.near_plane;
  const Vec3f gaze = camera.gaze;
  const float dist = camera.near_distance;
  const float l = view_plane.x;
  const float r = view_plane.y;
  const float b = view_plane.z;
  const float t = view_plane.w;
  const Vec3f v = camera.up;
  const Vec3f u = gaze.CrossProduct(v);

  const Vec3f m = camera.position + gaze*dist;
  const Vec3f q = m + u*l + v*t;

  const float su = (r-l)*(i+.5)/camera.image_width;
  const float sv = (t-b)*(j+.5)/camera.image_height;

  return q+u*su-v*sv;
}

Vec3i SceneRenderer::RenderPixel(int i, int j, const Camera& camera) {
  Vec3i color = scene_.background_color;
  int material_id = -1;
  float tmin = std::numeric_limits<float>::infinity();
  const Vec3f e = camera.position;
  const Vec3f s = CalculateS(i, j, camera);

  for(const Triangle& obj : scene_.triangles) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      material_id = obj.material_id;
    }
  }
  for(const Sphere& obj : scene_.spheres) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      material_id = obj.material_id;
    }
  }
  for(const Mesh& obj : scene_.meshes) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      material_id = obj.material_id;
    }
  }

  if(material_id!=-1) {
    color = scene_.ambient_light.PointWise(
	scene_.materials[material_id].ambient).ToVec3i();
  }

  return color;
}

Vec3i* SceneRenderer::RenderImage(const Camera& camera) {
  const int width = camera.image_width;
  const int height = camera.image_height;
  Vec3i* result = new Vec3i[width*height];

  for(int j=0;j<height;j++) {
    for(int i=0;i<width;i++) {
      result[j*width+i] = RenderPixel(i, j, camera);
    }
  }

  return result;
}
