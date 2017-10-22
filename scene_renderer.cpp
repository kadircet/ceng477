#include "scene_renderer.h"
using namespace parser;

Pixel* SceneRenderer::RenderImage(const Camera& camera) {
  const int width = camera.image_width;
  const int height = camera.image_height;
  Pixel* result = new Pixel[width*height];

  for(int i=0;i<width;i++) {
    for(int j=0;j<height;j++) {
      result[i*height+j] = RenderPixel(i, j, camera);
    }
  }

  return result;
}
