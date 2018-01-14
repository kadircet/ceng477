// Press Q to bring the light down.

#include <glm/glm.hpp>
#include "cameracontroller.h"
#include "helper.h"
#include "shadermanager.h"

static GLFWwindow* win = NULL;
GLuint shadow_frame;
GLuint depth_texture;
CameraController camera_controller;
CameraController light_vcs;
ShaderManager shadow_map_shader;
ShaderManager height_map_shader;
int width_win, height_win;
int old_width, old_height;
bool IsFullScreen;

constexpr const float kRotationDegrees = .1;
constexpr const float kSpeedIncrement = .1;

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/,
                        int action, int /*mods*/);

void InitOpenGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
}

const glm::mat4 bias_matrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0,
                            0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

void InitShadowMapping(const int width, const int height) {
  glGenFramebuffers(1, &shadow_frame);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_frame);

  glGenTextures(1, &depth_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depth_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depth_texture, 0);
  glDrawBuffer(GL_NONE);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "FAILED!!!" << std::endl;
  }
}

size_t InitFaces(const int width_texture, const int height_texture) {
  const size_t vertices_per_row = width_texture + 1;
  const size_t number_triangles = width_texture * height_texture * 2;
  size_t max_vertex_id = 0;
  GLuint* const indices = new GLuint[number_triangles * 3];
  for (size_t i = 0; i < number_triangles; i++) {
    const size_t grid_id = i / 2;
    const size_t row = grid_id / width_texture;
    const size_t col = grid_id % width_texture;
    const bool is_down = i % 2;
    const size_t upper_left_id = row * vertices_per_row + col;
    max_vertex_id = max(max_vertex_id, upper_left_id + vertices_per_row);
    if (is_down) {
      indices[3 * i] = upper_left_id + 1;
      indices[3 * i + 2] = upper_left_id + vertices_per_row + 1;
      max_vertex_id = max(max_vertex_id, upper_left_id + vertices_per_row + 1);
    } else {
      indices[3 * i] = upper_left_id;
      indices[3 * i + 2] = upper_left_id + 1;
    }
    indices[3 * i + 1] = upper_left_id + vertices_per_row;
  }
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, number_triangles * 3 * sizeof(GLuint),
               indices, GL_STATIC_DRAW);
  delete[] indices;

  return number_triangles;
}

void InitVertices(const int width_texture, const int height_texture) {
  const size_t vertices_per_row = width_texture + 1;
  const size_t vertices_count = vertices_per_row * (height_texture + 1);
  GLfloat* const vertices = new GLfloat[vertices_count * 3];
  for (size_t i = 0; i < vertices_count; i++) {
    vertices[3 * i] = i % vertices_per_row;
    vertices[3 * i + 1] = 0;
    vertices[3 * i + 2] = i / vertices_per_row;
  }
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices_count * 3, vertices,
               GL_STATIC_DRAW);
  delete[] vertices;

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
}

static void errorCallback(int error, const char* description) {
  fprintf(stderr, "Error: %d(%s)\n", error, description);
}

static void ResizeCallback(GLFWwindow*, int width, int height) {
  width_win = width;
  height_win = height;
}

void InitializeWindow() {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  width_win = height_win = 600;
  win = glfwCreateWindow(width_win, height_win, "CENG477 - HW4", NULL, NULL);

  if (!win) {
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(win);
  glfwSetKeyCallback(win, keyCallback);
  glfwSetWindowSizeCallback(win, ResizeCallback);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    glfwTerminate();
    exit(-1);
  }
}

static void keyCallback(GLFWwindow* window, int key, int /*scancode*/,
                        int action, int /*mods*/) {
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
      case GLFW_KEY_O:
        camera_controller.IncrementHeight();
        break;
      case GLFW_KEY_L:
        camera_controller.DecrementHeight();
        break;
      case GLFW_KEY_Q:
        light_vcs.SetPositionY(50);
        shadow_map_shader.UseShader();
        shadow_map_shader.Update("MVP", light_vcs.GetMVP());

        height_map_shader.UseShader();
        height_map_shader.Update("depthMVP", bias_matrix * light_vcs.GetMVP());
        height_map_shader.Update("lightPosition", light_vcs.GetPosition());
        break;
      case GLFW_KEY_A:
        camera_controller.ChangeYaw(-kRotationDegrees);
        break;
      case GLFW_KEY_D:
        camera_controller.ChangeYaw(kRotationDegrees);
        break;
      case GLFW_KEY_W:
        camera_controller.ChangePitch(kRotationDegrees);
        break;
      case GLFW_KEY_S:
        camera_controller.ChangePitch(-kRotationDegrees);
        break;
      case GLFW_KEY_U:
        camera_controller.IncrementSpeed(kSpeedIncrement);
        break;
      case GLFW_KEY_J:
        camera_controller.IncrementSpeed(-kSpeedIncrement);
        break;
      case GLFW_KEY_F:
        if (IsFullScreen) {
          width_win = old_width;
          height_win = old_height;
        } else {
          old_width = width_win;
          old_height = height_win;
          static GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
          const static GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
          width_win = mode->width;
          height_win = mode->height;
        }
        glfwSetWindowMonitor(window,
                             IsFullScreen ? NULL : glfwGetPrimaryMonitor(), 100,
                             100, width_win, height_win, GLFW_DONT_CARE);
        IsFullScreen ^= 1;
        break;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Please provide only a texture image\n");
    exit(-1);
  }

  InitializeWindow();
  InitOpenGL();

  int width_texture, height_texture;
  GLuint jpeg_texture = initTexture(argv[1], &width_texture, &height_texture);

  // Init geometry.
  InitVertices(width_texture, height_texture);
  const size_t number_triangles = InitFaces(width_texture, height_texture);
  const glm::vec3 position_light(
      width_texture / 2., width_texture + height_texture, height_texture / 2.);
  const glm::vec3 gaze(0, 0, 1);
  const glm::vec3 up(0, 1, 0);
  const glm::vec3 camera_position(width_texture / 2., width_texture / 10.,
                                  -width_texture / 4.);

  camera_controller =
      CameraController(camera_position, gaze, up, 45., 1., .1, 1000.);
  light_vcs = CameraController(position_light, -up, gaze, 179., 1., .1, 1000.);

  shadow_map_shader = ShaderManager("shadow.vert", "shadow.frag");
  shadow_map_shader.UseShader();
  shadow_map_shader.Update("widthTexture", width_texture);
  shadow_map_shader.Update("heightTexture", height_texture);
  shadow_map_shader.Update("MVP", light_vcs.GetMVP());

  InitShadowMapping(width_texture, height_texture);

  height_map_shader = ShaderManager("shader.vert", "shader.frag");
  height_map_shader.UseShader();
  height_map_shader.Update("widthTexture", width_texture);
  height_map_shader.Update("heightTexture", height_texture);
  height_map_shader.Update("depthMVP", bias_matrix * light_vcs.GetMVP());
  height_map_shader.Update("lightPosition", position_light);
  height_map_shader.Update("rgbTexture", 0);
  height_map_shader.Update("depthTexture", 1);

  while (glfwWindowShouldClose(win) == 0) {
    camera_controller.Move();

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_frame);
    glViewport(0, 0, width_texture, height_texture);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shadow_map_shader.UseShader();
    shadow_map_shader.Update("heightFactor",
                             camera_controller.GetHeightFactor());

    shadow_map_shader.Render(GL_TRIANGLES, number_triangles * 3,
                             GL_UNSIGNED_INT, static_cast<const void*>(0));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_win, height_win);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    height_map_shader.UseShader();
    height_map_shader.Update("MVP", camera_controller.GetMVP());
    height_map_shader.Update("MVIT", camera_controller.GetMVIT());
    height_map_shader.Update("MV", camera_controller.GetView());
    height_map_shader.Update("cameraPosition", camera_controller.GetPosition());
    height_map_shader.Update("heightFactor",
                             camera_controller.GetHeightFactor());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, jpeg_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    height_map_shader.Render(GL_TRIANGLES, number_triangles * 3,
                             GL_UNSIGNED_INT, static_cast<const void*>(0));

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  return 0;
}
