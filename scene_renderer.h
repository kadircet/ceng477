#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

#include "bounding_volume_hierarchy.h"
#include "parser.h"

class SceneRenderer {
 private:
  parser::Vec3f q, usu, vsv;
  parser::Scene scene_;
  std::vector<Object*> objects_;
  BoundingVolumeHierarchy* bounding_volume_hierarchy;

  float DoesIntersect(const parser::Vec3f& origin,
                      const parser::Vec3f& direction, const parser::Face& face);
  float DoesIntersect(const parser::Vec3f& origin,
                      const parser::Vec3f& direction, const parser::Mesh& mesh,
                      float tmax, const void* hit_obj);
  float DoesIntersect(const parser::Vec3f& origin,
                      const parser::Vec3f& direction, const parser::Mesh& mesh,
                      parser::Face const** intersecting_face);
  float DoesIntersect(const parser::Vec3f& origin,
                      const parser::Vec3f& direction,
                      const parser::Triangle& triangle);
  float DoesIntersect(const parser::Vec3f& origin,
                      const parser::Vec3f& direction,
                      const parser::Sphere& sphere);
  HitRecord GetIntersection(const Ray& ray);
  bool DoesIntersect(const Ray& ray, float tmax, const void* hit_obj);

  parser::Vec3f TraceRay(const Ray& ray, int depth, const Object* hit_obj);
  parser::Vec3f CalculateS(int i, int j);
  parser::Vec3i RenderPixel(int i, int j, const parser::Camera& camera);

 public:
  SceneRenderer(const char* scene_path);

  const std::vector<parser::Camera>& Cameras() { return scene_.cameras; }

  parser::Vec3i* RenderImage(const parser::Camera& camera);
};

#endif
