#include <iostream>
#include "parser.h"
#include "ppm.h"
#include "scene_renderer.h"

using namespace parser;

int main(int argc, char* argv[])
{
  SceneRenderer scene_renderer(argv[1]);

  for(const Camera& camera : scene_renderer.Cameras()) {
    const Pixel* pixels = scene_renderer.RenderImage(camera);
    const int width = camera.image_width;
    const int height = camera.image_height;
    unsigned char* image = new unsigned char[width*height*3];

    int idx = 0;
    for(int i=0;i<width;i++) {
      for(int j=0;j<height;j++) {
	const Pixel pixel = pixels[i*height+j];
	image[idx++] = pixel.r;
	image[idx++] = pixel.g;
	image[idx++] = pixel.b;
      }
    }
    delete [] image;
    delete [] pixels;

    write_ppm(argv[2], image, width, height);
  }
  return 0;
}
