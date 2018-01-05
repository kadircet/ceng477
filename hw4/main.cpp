#include <glm/glm.hpp>
#include "cameracontroller.h"
#include "helper.h"
#include "shadermanager.h"

static GLFWwindow* win = NULL;

void InitOpenGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
}

/*void InitShadowMapping(const int width, const int height,
                       const size_t number_triangles) {
  GLuint id_program_shader = initShaders("shadow.vert", "shadow.frag");

  const glm::vec3 position_light(width / 2., width + height, height / 4.);
  const glm::vec3 position(width / 2., width / 10., -width / 4.);
  const glm::vec3 gaze(0, 0, 1);
  const glm::mat4 view_matrix =
      glm::lookAt(position_light, position + gaze, glm::vec3(0, 1, 0));
  const glm::mat4 model_matrix = glm::mat4(1.0f);

  const glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;
  GLuint idMVPMatrix = glGetUniformLocation(id_program_shader, "MVP");
  glUniformMatrix4fv(idMVPMatrix, 1, GL_FALSE, &MVP[0][0]);

  const GLuint width_idx =
      glGetUniformLocation(id_program_shader, "widthTexture");
  glUniform1i(width_idx, width);

  const GLuint height_idx =
      glGetUniformLocation(id_program_shader, "heightTexture");
  glUniform1i(height_idx, height);

  const GLuint heigt_factor_idx =
      glGetUniformLocation(id_program_shader, "heightFactor");
  glUniform1f(heigt_factor_idx, 10.);

  GLuint shadow_buffer;
  glGenFramebuffers(1, &shadow_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffer);

  GLuint depthTexture;
  glGenTextures(1, &depthTexture);
  glBindTexture(GL_TEXTURE_2D, depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
  glDrawBuffer(GL_NONE);

  glViewport(0, 0, width, height);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(id_program_shader);
  glDrawElements(GL_TRIANGLES, number_triangles * 3, GL_UNSIGNED_INT,
                 static_cast<void*>(0));

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, 600, 600);
}*/

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
  std::cout << "NumberTriangles: " << number_triangles << std::endl;
  std::cout << "MaxVertexId: " << max_vertex_id << std::endl;

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
  std::cout << "NumberVertices: " << vertices_count << std::endl;
}

static void errorCallback(int error, const char* description) {
  fprintf(stderr, "Error: %d(%s)\n", error, description);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Please provide only a texture image\n");
    exit(-1);
  }

  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  win = glfwCreateWindow(600, 600, "CENG477 - HW4", NULL, NULL);

  if (!win) {
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(win);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    glfwTerminate();
    exit(-1);
  }

  InitOpenGL();

  int width_texture, height_texture;
  initTexture(argv[1], &width_texture, &height_texture);

  // Init geometry.
  InitVertices(width_texture, height_texture);
  const size_t number_triangles = InitFaces(width_texture, height_texture);

  CameraController camera_controller(
      glm::vec3(width_texture / 2., width_texture / 10., -width_texture / 4.),
      glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), 45., 1., .1, 1000.);

  ShaderManager height_map_shader("shader.vert", "shader.frag");
  height_map_shader.UseShader();
  height_map_shader.Update("widthTexture", width_texture);
  height_map_shader.Update("heightTexture", height_texture);

  while (!glfwWindowShouldClose(win)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    height_map_shader.Update("MVP", camera_controller.GetMVP());
    height_map_shader.Update("MVIT", camera_controller.GetMVIT());
    height_map_shader.Update("MV", camera_controller.GetView());
    height_map_shader.Update("cameraPosition", camera_controller.GetPosition());
    height_map_shader.Update("heightFactor",
                             camera_controller.GetHeightFactor());
    height_map_shader.Render(GL_TRIANGLES, number_triangles * 3,
                             GL_UNSIGNED_INT, static_cast<const void*>(0));

    glfwSwapBuffers(win);
    glfwWaitEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  return 0;
}
