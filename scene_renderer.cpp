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
    Vec3f centerOfSphere = scene_.vertex_data[sphere.center_vertex_id];
    Vec3f sphereToCamera = e - centerOfSphere;
    const float distanceTimesSphereToCamera = distance * sphereToCamera;
    const float normOfDistanceSquared = distance * distance;
    const float normOfSphereToCameraSquared = sphereToCamera * sphereToCamera;
    const float radiusSquared = sphere.radius * sphere.radius;  
    return 0.0f;
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
