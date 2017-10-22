#include <limits>
#include "scene_renderer.h"

using namespace parser;

constexpr const Vec3i red(255,0,0);

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Mesh& mesh) {
    return false;
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, cont Triangle& triangle) {
    return false;
}

float SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Sphere& sphere) {
    return false;
}

Vec3i SceneRenderer::RenderPixel(int i, int j, const Camera& camera) {
  Vec3i color = scene_.background_color;
  float tmin = std::numeric_limits<float>::infinity();
  const Vec3f e = camera.position;
  const Vec3f s = CalculateS(i, j, camera);

  for(const Triangle& obj : triangles) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      color = red;
    }
  }
  for(const Sphere& obj : spheres) {
    float t = DoesIntersect(e, s, obj);
    if(t<tmin) {
      tmin = t;
      color = red;
    }
  }
  for(const Mesh& obj : meshes) {
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
