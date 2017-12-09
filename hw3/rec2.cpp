/*
    CENG477 - Introduction to Computer Graphics
    Recitation 2
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

static GLFWwindow* win = NULL;

static float vertices[] = {
    0.0, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, -0.5, 0.0,
};

static float colors[] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};

// The function below can be used for human-readable description of possible
// future GLFW errors.
static void errorCallback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                        int mods) {
  /*
      GLFW window key handler.
      Descriptions of keys are defined in GLFW itself.
      Here is a list of them: http://www.glfw.org/docs/latest/group__keys.html
  */

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void RenderFunction1() {
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Anchient method to draw primitives;
  // The order of vertices are also important!!!

  glBegin(GL_TRIANGLES);

  glColor3f(colors[0], colors[1], colors[2]);
  glVertex3f(vertices[0], vertices[1], vertices[2]);

  glColor3f(colors[3], colors[4], colors[5]);
  glVertex3f(vertices[3], vertices[4], vertices[5]);

  glColor3f(colors[6], colors[7], colors[8]);
  glVertex3f(vertices[6], vertices[7], vertices[8]);

  glEnd();
}

int main(int argc, char* argv[]) {
  glfwSetErrorCallback(errorCallback);

  /*
      glfwInit() and glfwTerminate() are must function for allocation, and
     relase of necassary resources.
  */
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  /*
      glfwInit() function sets window and OpenGL context properties to its
     default values. glfwWindowHint() function provides opportunity to modify
     some of the window, and OpenGL properties. In the usage below, the OpenGL
     version is set to 3.1 which will be used during assignment.

      For more info: http://www.glfw.org/docs/latest/window_guide.html
  */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  /*
      GLFW windows can be stored in GLFWwindow object.
      After GLFW initilization, its window can created with glfwCreateWindow()
     function. Its parameters are width, height, window name, monitor, and
     share, respectively. Monitor is used for full screen GLFWwindow, and share
     is used for sharing objects between different OpenGL contexts.

      For more info: http://www.glfw.org/docs/latest/window_guide.html
  */
  win = glfwCreateWindow(640, 480, "Recitation 2", NULL, NULL);

  if (!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Necessary for handling keyboard operations.
  glfwSetKeyCallback(win, keyCallback);

  // Function selects the window that objects will be rendered to.
  glfwMakeContextCurrent(win);

  while (!glfwWindowShouldClose(win)) {
    glfwWaitEvents();  // Waiting for events to be process, like keyboard
                       // events, mouse events, etc.

    RenderFunction1();

    glfwSwapBuffers(win);  // Double buffering
  }

  glfwDestroyWindow(win);
  glfwTerminate();

  exit(EXIT_SUCCESS);
  return 0;
}
