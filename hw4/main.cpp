#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper.h"

static GLFWwindow* win = NULL;

// Shaders
GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;
GLuint idJpegTexture;
GLuint idMVPMatrix;
GLuint idMVMatrix;
GLuint idMVITMatrix;

void InitOpenGL() {
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
}

void InitMVP(const int width) {
#ifdef GLM_FORCE_RADIANS
  const glm::mat4 projection_matrix =
      glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
#else
  const glm::mat4 projection_matrix =
      glm::perspective(45.0f, 1.0f, 0.1f, 1000.0f);
#endif
  const glm::vec3 position(width / 2., width / 10., -width / 4.);
  const glm::vec3 gaze(0, 0, 1);
  const glm::mat4 view_matrix =
      glm::lookAt(position, position + gaze, glm::vec3(0, 1, 0));
  const glm::mat4 model_matrix = glm::mat4(1.0f);

  const glm::mat4 MVP = projection_matrix * view_matrix * model_matrix;
  idMVPMatrix = glGetUniformLocation(idProgramShader, "MVP");
  glUniformMatrix4fv(idMVPMatrix, 1, GL_FALSE, &MVP[0][0]);

  const glm::mat4 MV = view_matrix * model_matrix;
  idMVMatrix = glGetUniformLocation(idProgramShader, "MV");
  glUniformMatrix4fv(idMVMatrix, 1, GL_FALSE, &MV[0][0]);

  const glm::mat4 MVIT = glm::inverseTranspose(MV);
  idMVITMatrix = glGetUniformLocation(idProgramShader, "MVIT");
  glUniformMatrix4fv(idMVITMatrix, 1, GL_FALSE, &MVIT[0][0]);
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
  idProgramShader = initShaders("shader.vert", "shader.frag");
  glUseProgram(idProgramShader);
  int widthTexture, heightTexture;
  initTexture(argv[1], &widthTexture, &heightTexture);
  const GLuint width_idx =
      glGetUniformLocation(idProgramShader, "widthTexture");
  glUniform1i(width_idx, widthTexture);
  const GLuint height_idx =
      glGetUniformLocation(idProgramShader, "heightTexture");
  glUniform1i(height_idx, heightTexture);
  const GLuint heigt_factor_idx =
      glGetUniformLocation(idProgramShader, "heightFactor");
  glUniform1f(heigt_factor_idx, 10.);
  const GLuint camera_position_idx =
      glGetUniformLocation(idProgramShader, "cameraPosition");
  std::cout << "CameraID: " << camera_position_idx << std::endl;
  const glm::vec3 position(widthTexture / 2., widthTexture / 10.,
                           -widthTexture / 4.);
  glUniform4f(camera_position_idx, position.x, position.y, position.z, 1.);

  InitMVP(widthTexture);
  InitVertices(widthTexture, heightTexture);
  const size_t number_triangles = InitFaces(widthTexture, heightTexture);

  while (!glfwWindowShouldClose(win)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, number_triangles * 6, GL_UNSIGNED_INT,
                   static_cast<void*>(0));
    glfwSwapBuffers(win);
    glfwWaitEvents();
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  return 0;
}
