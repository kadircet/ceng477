#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

#include "parser.h"

struct Ray {
  parser::Vec3f origin;
  parser::Vec3f direction;
};

struct HitRecord {
  int material_id;
  float t;
  parser::Vec3f normal;
};

class SceneRenderer {
private:
  parser::Vec3f q, usu, vsv;
  parser::Scene scene_;

  float DoesIntersect(const parser::Vec3f &origin,
                      const parser::Vec3f &direction, const parser::Face &face);
  float DoesIntersect(const parser::Vec3f &origin,
                      const parser::Vec3f &direction, const parser::Mesh &mesh);
  float DoesIntersect(const parser::Vec3f &origin,
                      const parser::Vec3f &direction, const parser::Mesh &mesh,
                      parser::Face &intersecting_face);
  float DoesIntersect(const parser::Vec3f &origin,
                      const parser::Vec3f &direction,
                      const parser::Triangle &triangle);
  float DoesIntersect(const parser::Vec3f &origin,
                      const parser::Vec3f &direction,
                      const parser::Sphere &sphere);
  HitRecord GetIntersection(const Ray &ray);
  bool DoesIntersect(const Ray &ray, float tmax);

  parser::Vec3f TraceRay(const Ray &ray);
  parser::Vec3f CalculateS(int i, int j);
  parser::Vec3i RenderPixel(int i, int j, const parser::Camera &camera);

public:
  SceneRenderer(const char *scene_path) { scene_.loadFromXml(scene_path); }

  const std::vector<parser::Camera> &Cameras() { return scene_.cameras; }

  parser::Vec3i *RenderImage(const parser::Camera &camera);
};

#endif
