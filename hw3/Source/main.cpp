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
    delete[] vertex_pos;

    // Store indices.
    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    GLuint* indices = new GLuint[scene.face_count * 3];
    int indices_idx = 0;
    const std::vector<parser::Mesh> meshes = scene.meshes;
    for (const parser::Mesh& mesh : meshes) {
      const std::vector<parser::Face> faces = mesh.faces;
      for (const parser::Face& face : faces) {
        indices[indices_idx++] = face.v0_id;
        indices[indices_idx++] = face.v1_id;
        indices[indices_idx++] = face.v2_id;
      }
    }
    int indexDataSizeInBytes = scene.face_count * 3 * sizeof(GLuint);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indices,
                 GL_STATIC_DRAW);
    delete[] indices;
  }

  glVertexPointer(3, GL_FLOAT, 0, 0);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glPushMatrix();
  glTranslatef(scene.translations[2].x, scene.translations[2].y,
               scene.translations[2].z);
  glRotatef(scene.rotations[0].x, scene.rotations[0].y, scene.rotations[0].z,
            scene.rotations[0].w);
  glRotatef(scene.rotations[0].x, scene.rotations[0].y, scene.rotations[0].z,
            scene.rotations[0].w);
  glRotatef(scene.rotations[0].x, scene.rotations[0].y, scene.rotations[0].z,
            scene.rotations[0].w);
  glDrawElements(GL_TRIANGLES, scene.face_count * 3 - 6, GL_UNSIGNED_INT,
                 (const void*)(6 * sizeof(GLuint)));
  glPopMatrix();
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
  const Vec3f center = eye + gaze * camera.near_distance;
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

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
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
