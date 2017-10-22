#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

struct Pixel {
  unsigned char r,g,b;
};

class SceneRenderer {
private:
  parser::Scene scene_;

  bool DoesIntersect(const Vec3f& e, const Vec3f& s, const Mesh& mesh);
  bool DoesIntersect(const Vec3f& e, const Vec3f& s, const Triangle& triangle);
  bool DoesIntersect(const Vec3f& e, const Vec3f& s, const Sphere& sphere);

  Vec3f CalculateS(int i, int j, const parser::Camera& camera);

  Pixel RenderPixel(int i, int j, const parser::Camera& camera);
  
public:
  SceneRenderer(const string& scene_path) {
    scene_.loadFromXml(scene_path);
  }

  const std::vector<parser::Camera>& Cameras() {
    return scene.cameras;
  }

  Pixel* RenderImage(const parser::Camera& camera);
};

#endif
