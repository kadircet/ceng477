#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

#include "parser.h"

struct Pixel {
  unsigned char r,g,b;
};

class SceneRenderer {
private:
  parser::Scene scene_;

  bool DoesIntersect(const parser::Vec3f& e,
      const parser::Vec3f& s, const parser::Mesh& mesh);
  bool DoesIntersect(const parser::Vec3f& e,
      const parser::Vec3f& s, const parser::Triangle& triangle);
  bool DoesIntersect(const parser::Vec3f& e,
      const parser::Vec3f& s, const parser::Sphere& sphere);

  parser::Vec3f CalculateS(int i, int j, const parser::Camera& camera);

  Pixel RenderPixel(int i, int j, const parser::Camera& camera);
  
public:
  SceneRenderer(const char* scene_path) {
    scene_.loadFromXml(scene_path);
  }

  const std::vector<parser::Camera>& Cameras() {
    return scene_.cameras;
  }

  Pixel* RenderImage(const parser::Camera& camera);
};

#endif
