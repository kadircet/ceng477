#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "parser.h"

using parser::Vec3f;
// Sample usage for reading an XML scene file
parser::Scene scene;
static GLFWwindow* win = NULL;

char gRendererInfo[512] = {0};
char gWindowTitle[512] = {0};

int gWidth, gHeight;
void Init() {
  glEnable(GL_DEPTH_TEST);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glShadeModel(GL_FLAT);
}

static void errorCallback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                        int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void display() {
  static bool firstTime = true;

  static int vertexPosDataSizeInBytes;

  if (firstTime) {
    firstTime = false;

    glEnableClientState(GL_VERTEX_ARRAY);

    // Store vertex data.
    GLuint vertexAttribBuffer;
    const std::vector<Vec3f> vertex_data = scene.vertex_data;
    GLfloat* vertex_pos = new GLfloat[vertex_data.size() * 3];
    int idx = 0;
    for (const Vec3f& vertex : vertex_data) {
      vertex_pos[idx++] = vertex.x;
      vertex_pos[idx++] = vertex.y;
      vertex_pos[idx++] = vertex.z;
    }
    glGenBuffers(1, &vertexAttribBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexAttribBuffer);
    vertexPosDataSizeInBytes = vertex_data.size() * 3 * sizeof(GLfloat);
    glBufferData(GL_ARRAY_BUFFER, vertexPosDataSizeInBytes, vertex_pos,
                 GL_STATIC_DRAW);

    const std::vector<parser::Mesh> meshes = scene.meshes;
    const parser::Mesh mesh = scene.meshes[0];
    {
      const std::vector<parser::Face> faces = mesh.faces;
      GLuint index_buffer;
      GLuint* indices = new GLuint[faces.size() * 3];
      int indices_idx = 0;
      for (const parser::Face& face : faces) {
        indices[indices_idx++] = face.v0_id;
        indices[indices_idx++] = face.v1_id;
        indices[indices_idx++] = face.v2_id;
      }
      glGenBuffers(1, &index_buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
      int indexDataSizeInBytes = sizeof(faces.size() * 3 * sizeof(GLuint));
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indices,
                   GL_STATIC_DRAW);
    }
    /*firstTime = false;
    glEnableClientState(GL_VERTEX_ARRAY);
    GLuint indices[] = {
        0, 1, 2,  // front
        3, 0, 2,  // front
        4, 7, 6,  // back
        5, 4, 6,  // back
        0, 3, 4,  // left
        3, 7, 4,  // left
        2, 1, 5,  // right
        6, 2, 5,  // right
        3, 2, 7,  // top
        2, 6, 7,  // top
        0, 4, 1,  // bottom
        4, 5, 1   // bottom
    };
    GLfloat vertexPos[] = {
        -0.5, -0.5, 0.5,   // 0: bottom-left-front
        0.5,  -0.5, 0.5,   // 1: bottom-right-front
        0.5,  0.5,  0.5,   // 2: top-right-front
        -0.5, 0.5,  0.5,   // 3: top-left-front
        -0.5, -0.5, -0.5,  // 4: bottom-left-back
        0.5,  -0.5, -0.5,  // 5: bottom-right-back
        0.5,  0.5,  -0.5,  // 6: top-right-back
        -0.5, 0.5,  -0.5,  // 7: top-left-back
    };
    GLuint vertexAttribBuffer, indexBuffer;
    glGenBuffers(1, &vertexAttribBuffer);
    glGenBuffers(1, &indexBuffer);
    assert(vertexAttribBuffer > 0 && indexBuffer > 0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    vertexPosDataSizeInBytes = sizeof(vertexPos);
    int indexDataSizeInBytes = sizeof(indices);
    glBufferData(GL_ARRAY_BUFFER, vertexPosDataSizeInBytes, vertexPos,
                 GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indices,
                 GL_STATIC_DRAW);*/
  }

  glVertexPointer(3, GL_FLOAT, 0, 0);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void render() {
  static int framesRendered = 0;
  static std::chrono::time_point<std::chrono::system_clock> start =
      std::chrono::system_clock::now();

  glClearColor(scene.background_color.x, scene.background_color.y,
               scene.background_color.z, 1);
  glClearDepth(1.0f);
  glClearStencil(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  ++framesRendered;
  display();

  std::chrono::time_point<std::chrono::system_clock> end =
      std::chrono::system_clock::now();
  std::chrono::duration<double> elapsedTime = end - start;
  if (elapsedTime.count() > 1.) {
    start = std::chrono::system_clock::now();

    std::stringstream stream;
    stream << framesRendered;
    framesRendered = 0;

    strcpy(gWindowTitle, gRendererInfo);
    strcat(gWindowTitle, " - ");
    strcat(gWindowTitle, stream.str().c_str());
    strcat(gWindowTitle, " FPS");
    std::cout << gWindowTitle << std::endl;
  }
}

void SetCamera() {
  const parser::Camera camera = scene.camera;
  const Vec3f eye = camera.position;
  const Vec3f gaze = camera.gaze.Normalized();
  const Vec3f u = gaze.CrossProduct(camera.up).Normalized();
  const Vec3f v = u.CrossProduct(gaze);
  const Vec3f center = camera.position + gaze * camera.near_distance;
  const parser::Vec4f near_plane = camera.near_plane;
  gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, v.x, v.y, v.z);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(near_plane.x, near_plane.y, near_plane.z, near_plane.w,
            camera.near_distance, camera.far_distance);
  glMatrixMode(GL_MODELVIEW);
}

void reshape(GLFWwindow* win, int w, int h) {
  w = w < 1 ? 1 : w;
  h = h < 1 ? 1 : h;

  gWidth = w;
  gHeight = h;

  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, 1, 1, 500);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "BOK YE OYLE SEY MI OLUR" << std::endl;
    exit(1);
  }
  scene.loadFromXml(argv[1]);

  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  win = glfwCreateWindow(scene.camera.image_width, scene.camera.image_height,
                         "CENG477", NULL, NULL);
  if (!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(win);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(win, keyCallback);
  glfwSetWindowSizeCallback(win, reshape);

  Init();
  SetCamera();
  // reshape(win, 640, 480);
  while (!glfwWindowShouldClose(win)) {
    glfwPollEvents();
    render();
    glfwSwapBuffers(win);
  }
  glfwDestroyWindow(win);
  glfwTerminate();

  exit(EXIT_SUCCESS);

  return 0;
}
