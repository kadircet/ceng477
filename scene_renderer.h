#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

#include "parser.h"

class SceneRenderer {
private:
  parser::Scene scene_;

  float DoesIntersect(const parser::Vec3f &e, const parser::Vec3f &s,
                      const parser::Face &face);
  float DoesIntersect(const parser::Vec3f &e, const parser::Vec3f &s,
                      const parser::Mesh &mesh,
                      parser::Face &intersecting_face);
  float DoesIntersect(const parser::Vec3f &e, const parser::Vec3f &s,
                      const parser::Triangle &triangle);
  float DoesIntersect(const parser::Vec3f &e, const parser::Vec3f &s,
                      const parser::Sphere &sphere);

  parser::Vec3f CalculateS(int i, int j, const parser::Camera &camera);

  parser::Vec3i RenderPixel(int i, int j, const parser::Camera &camera);

public:
  SceneRenderer(const char *scene_path) { scene_.loadFromXml(scene_path); }

  const std::vector<parser::Camera> &Cameras() { return scene_.cameras; }

  parser::Vec3i *RenderImage(const parser::Camera &camera);
};

#endif
