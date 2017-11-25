#include <iostream>
#include <thread>
#include "parser.h"
#include "ppm.h"
#include "scene_renderer.h"
using namespace parser;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "please provide scene file" << std::endl;
    return 1;
  }
  SceneRenderer scene_renderer(argv[1]);

  for (const Camera& camera : scene_renderer.Cameras()) {
    const int width = camera.image_width;
    const int height = camera.image_height;
    Vec3i* pixels = new Vec3i[width * height];
    const int number_of_cores = 128;
    scene_renderer.SetUpScene(camera);
    if (number_of_cores == 0 || height < number_of_cores) {
      scene_renderer.RenderImage(camera, pixels, 0, height, width);
    } else {
      std::thread* threads = new std::thread[number_of_cores];
      const int height_increase = height / number_of_cores;
      for (int i = 0; i < number_of_cores; i++) {
        const int min_height = i * height_increase;
        const int max_height =
            (i == number_of_cores - 1) ? height : (i + 1) * height_increase;
        threads[i] = std::thread(&SceneRenderer::RenderImage, &scene_renderer,
                                 camera, pixels, min_height, max_height, width);
      }
      for (int i = 0; i < number_of_cores; i++) threads[i].join();
      delete[] threads;
    }
    unsigned char* image = new unsigned char[width * height * 3];

    int idx = 0;
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        const Vec3i pixel = pixels[i * height + j];
        image[idx++] = pixel.x;
        image[idx++] = pixel.y;
        image[idx++] = pixel.z;
      }
    }
    delete[] pixels;
    write_ppm(camera.image_name.c_str(), image, width, height);
    delete[] image;
  }
  return 0;
}
