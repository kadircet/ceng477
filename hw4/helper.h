#ifndef __HELPER__H__
#define __HELPER__H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <string>
// clang-format off
#include <jpeglib.h>
// clang-format off

using namespace std;

GLuint initShaders(const string&, const string&);
GLuint initVertexShader(const string& filename);
GLuint initFragmentShader(const string& filename);
bool readDataFromFile(const string& fileName, string& data);
GLuint initTexture(char* filename, int* w, int* h);

#endif
