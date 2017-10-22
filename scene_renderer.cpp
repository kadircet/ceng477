#include <limits>
#include <cmath>
#include "scene_renderer.h"

using namespace parser;

constexpr const Vec3i red{255,0,0};

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
  float tmin = std::numeric_limits<float>::infinity();
  const Vec3f e = camera.position;
  const Vec3f s = CalculateS(i, j, camera);

  for(const Triangle& obj : scene_.triangles) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      color = red;
    }
  }
  for(const Sphere& obj : scene_.spheres) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      color = red;
    }
  }
  for(const Mesh& obj : scene_.meshes) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      color = red;
    }
  }

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
