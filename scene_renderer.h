#ifndef _SCENE_RENDERER_H
#define _SCENE_RENDERER_H

class SceneRenderer {
private:
  parser::Scene scene;
public:
  SceneRenderer(const string& scene_path) {
    scene.loadFromXml(scene_path);
  }

  const std::vector<parser::Camera>& Cameras() {
    return scene.cameras;
  }
};

#endif
