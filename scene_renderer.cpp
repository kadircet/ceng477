#include <limits>
#include <cmath>
#include "scene_renderer.h"

using namespace parser;

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Mesh& mesh) {
    return 0.0f;
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Triangle& triangle) {
    return 0.0f;
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Sphere& sphere) {
    Vec3f distance = s - e;
    Vec3f center_of_sphere = scene_.vertex_data[sphere.center_vertex_id];
    Vec3f sphere_to_camera = e - center_of_sphere;
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

Vec3i SceneRenderer::RenderPixel(int i, int j, const Camera& camera) {
  Vec3i color = scene_.background_color;
  float tmin = std::numeric_limits<float>::infinity();

  return color;
}

Vec3i* SceneRenderer::RenderImage(const Camera& camera) {
  const int width = camera.image_width;
  const int height = camera.image_height;
  Vec3i* result = new Vec3i[width*height];

  for(int i=0;i<width;i++) {
    for(int j=0;j<height;j++) {
      result[i*height+j] = RenderPixel(i, j, camera);
    }
  }

  return result;
}
